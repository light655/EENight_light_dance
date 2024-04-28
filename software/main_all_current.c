#include <stdio.h>
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "pico/sync.h"

// define variables shared between cores
static mutex_t intensity_mutex;
float intensities[4];           // intensity of the 4 channels in amp
#include "setting1.h"

// main function to run on core 1, defined in core1.c
// core 1 deals with setting the light intensity according the array
// #include "core1_old.h"
#include "core1.h"
#include "helper.h"
#include "pindef.h"

#define CURRENT_MARGIN 0.01f    // current control margin of 0.01A
#define VBAT_LOW 3.0f           // VBAT low threshold of 3.0V
#define CONTROL_PERIOD 150      // period of control commands in us

// float MA_array[4][499] = {0.0f};

// main function to run on core 0
// core 0 deals analogue sensing, feedback control, and emergency cutoff
int main(void) {
    // STDIO initialisation ---------------------------------------------------
    // use uart0 at GPIO 16, 17 for communication
    stdio_uart_init_full(uart0, PICO_DEFAULT_UART_BAUD_RATE, 16, 17);

    multicore_launch_core1(core1_entry);            // launch core 1

    // SPI initialisation -----------------------------------------------------
    spi_init(spi0, 100 * 1000);                     // 100kHz SPI
    spi_set_format(spi0, 8, 0, 0, SPI_MSB_FIRST);   // 8-bit, spi mode 0
    gpio_set_function(SPI_CLK, GPIO_FUNC_SPI);
    gpio_set_function(SPI_TX, GPIO_FUNC_SPI);

    SPI_CS_mask = (1 << SPI_CS0) | (1 << SPI_CS1) | (1 << SPI_CS2) | (1 << SPI_CS3);
    gpio_init_mask(SPI_CS_mask);
    gpio_set_dir_out_masked(SPI_CS_mask);
    gpio_set_mask(SPI_CS_mask);             // set all SPI CS pins high

    // LDO enable pin initialisation ------------------------------------------
    LDO_EN_mask = (1 << LDO_EN0) | (1 << LDO_EN1) | (1 << LDO_EN2) | (1 << LDO_EN3);
    gpio_init_mask(LDO_EN_mask);
    gpio_set_dir_out_masked(LDO_EN_mask);
    gpio_clr_mask(LDO_EN_mask);             // set all LDO EN pins low
    
    // ADC and MUX initialisation ---------------------------------------------
    adc_init();
    adc_gpio_init(VC_SENSE);
    adc_gpio_init(VL_SENSE);
    adc_gpio_init(VBAT_SENSE);
    MUX_mask = (1 << MUX_S0) | (1 << MUX_S1);
    gpio_init_mask(MUX_mask);
    gpio_set_dir_out_masked(MUX_mask);

    // main code --------------------------------------------------------------
    uint16_t VC[4], VL[4];      // arrays to store VC and VL of each channel
    int RDAC_N[4] = {0};        // array to store RDAC value of each channel
    float I_avg[4] = {0.0f};    // array to store average current of each channel
    float I_avg_prev[4] = {0.0f};         
    float I_target[4] = {0.0f}; // array to store target current of each channel
    // int MA_index[4] = {0};
    
    float adj;
    uint16_t vbat_16;

    sleep_ms(1000);     // wait for 1 second for the voltages to settle from startup

    uint64_t t1, t2;
    t2 = to_us_since_boot(get_absolute_time());

    while(true) {
        t1 = to_us_since_boot(get_absolute_time());
        while(t1 - t2 < CONTROL_PERIOD) {
            tight_loop_contents();
            t1 = to_us_since_boot(get_absolute_time());
        }
        t2 = t1;

        // update target if available 
        if(mutex_try_enter(&intensity_mutex, NULL)) {
            I_target[0] = intensities[0];
            I_target[1] = intensities[1];
            I_target[2] = intensities[2];
            I_target[3] = intensities[3];
            mutex_exit(&intensity_mutex);
        }

        // read current sense voltage
        VC_read(&VC[0], 0);
        VC_read(&VC[1], 1);
        VC_read(&VC[2], 2);
        VC_read(&VC[3], 3);

        // // moving average filter, bin=100
        // I_avg[0] = I_avg[0] * 0.998 + (float)VC[0] / 4096.0 * 3.3 / 4.0 * 0.002;
        // I_avg[1] = I_avg[1] * 0.998 + (float)VC[1] / 4096.0 * 3.3 / 4.0 * 0.002;
        // I_avg[2] = I_avg[2] * 0.998 + (float)VC[2] / 4096.0 * 3.3 / 4.0 * 0.002;
        // I_avg[3] = I_avg[3] * 0.998 + (float)VC[3] / 4096.0 * 3.3 / 4.0 * 0.002;

        // feedback control
        for(int i = 0; i < 4; i++) {
            // only control when I_target is higher than the minimum
            if(I_target[i] > current_min[i]) {
                I_avg[i] = I_avg[i] * 0.998 + (float)VC[i] / 4096.0 * 3.3 / 4.0 * 0.002;
                // tmp = (float)VC[i] / 4096.0 * 3.3 / 4.0 / 499.0;
                // I_avg[i] = I_avg[i] - MA_array[i][MA_index[i]] + tmp;
                // MA_array[i][MA_index[i]] = tmp;
                // if(MA_index[i] >= 499) MA_index[i] = 0;
                // else MA_index[i]++;

                adj = ((I_avg[i] - I_target[i]) * control_gain[i]) + ((I_avg[i] - I_avg_prev[i]) * derivative_gain[i]);
                I_avg_prev[i] = I_avg[i];

                RDAC_N[i] -= (int)adj;
                if(RDAC_N[i] > 255) RDAC_N[i] = 255;
                if(RDAC_N[i] < 0) RDAC_N[i] = 0;

                // if(i == 2) {
                //     printf("I_avg[2] = %f, I_tar[2] = %f\n", I_avg[2], I_target[2]);
                // }

                gpio_put(LDO_EN0 + 2 * i, 1);       // enable LDO
                gpio_put(PWM_PIN0 + 2 * i, 1);
                RDAC_set(SPI_CS0 + i, RDAC_N[i]);   // set RDAC register
            } else {
                gpio_put(LDO_EN0 + 2 * i, 0);       // disable LDO
                gpio_put(PWM_PIN0 + 2 * i, 0);
                RDAC_N[i] = 0;
            }
        }
/*
        // TODO: check battery voltage
        adc_select_input(2);
        vbat_16 = adc_read();
        if(vbat_16 < 2000) {    // end the loop if VBAT drop below 3.3V
            gpio_clr_mask(LDO_EN_mask);
            gpio_put(LED_R, 1);
            break;
        }
        */
    }

    return 0;
}

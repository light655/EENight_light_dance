#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"

#define LDO_EN0 1
#define LDO_EN1 3
#define LDO_EN2 5
#define LDO_EN3 7
#define PWM_PIN0 0
#define PWM_PIN1 2
#define PWM_PIN2 4
#define PWM_PIN3 6
#define SPI_CS0 10
#define SPI_CS1 11
#define SPI_CS2 12
#define SPI_CS3 13
#define SPI_CLK 18
#define SPI_TX 19
#define MUX_S0 20
#define MUX_S1 21
#define VC_SENSE 26
#define VL_SENSE 27
#define VBAT_SENSE 28
#define LED_R 8
#define LED_G 9
#define LED_PICO 25
#define T_SYNC 14

inline static void RDAC_set(uint CS_pin, uint8_t val);
inline static void MUX_select(uint channel);
inline static void read_voltage(uint16_t *voltage, uint channel, uint type);

uint SPI_CS_mask, PWM_mask, LDO_EN_mask, MUX_mask;

int main(void) {
    // STDIO initialisation ---------------------------------------------------
    stdio_uart_init_full(uart0, PICO_DEFAULT_UART_BAUD_RATE, 16, 17);
        // use uart0 at GPIO 16, 17 for communication

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

    // PWM initialisation -----------------------------------------------------
    PWM_mask = (1 << PWM_PIN0) | (1 << PWM_PIN1) | (1 << PWM_PIN2) | (1 << PWM_PIN3); 
    gpio_init_mask(PWM_mask);
    gpio_set_dir_out_masked(PWM_mask);
    gpio_clr_mask(PWM_mask);
    // gpio_set_function(PWM_PIN0, GPIO_FUNC_PWM);
    // gpio_set_function(PWM_PIN1, GPIO_FUNC_PWM);
    // gpio_set_function(PWM_PIN2, GPIO_FUNC_PWM);
    // gpio_set_function(PWM_PIN3, GPIO_FUNC_PWM);
        // not complete
    
    // ADC and MUX initialisation ---------------------------------------------
    adc_init();
    adc_gpio_init(VC_SENSE);
    adc_gpio_init(VL_SENSE);
    adc_gpio_init(VBAT_SENSE);
    MUX_mask = (1 << MUX_S0) | (1 << MUX_S1);
    gpio_init_mask(MUX_mask);
    gpio_set_dir_out_masked(MUX_mask);

    // LED initialisation -----------------------------------------------------
    uint LED_mask = (1 << LED_G) | (1 << LED_R) | (1 << LED_PICO);
    gpio_init_mask(LED_mask);
    gpio_set_dir_out_masked(LED_mask);

    // main code --------------------------------------------------------------
    gpio_put(LDO_EN1, 1);       // enable LDO1 for testing
    gpio_put(PWM_PIN1, 1);      // turn on NMOS1 for testing

    uint16_t VL1, VC1;
    float I1_avg = 0;
    const float target = 0.15;
    const float margin = 0.01;
    uint8_t N1 = 128;
    int x = 0;
    while(true) {
        /*
        for(uint8_t i = 64; i <= 192; i += 8) {
            printf("RDAC wiper: %d\n", i);
            RDAC_set(SPI_CS1, i);
            sleep_ms(2000);         // wait for output voltage to settle
            read_voltage(&VL1, 1, VL_SENSE);
            read_voltage(&VC1, 1, VC_SENSE);
            printf("Voltage at channel 1 output is %u = %fV.\n", VL1, (float)VL1 / 4096.0 * 3.3);
            printf("Current at channel 1 output is %u = %fA.\n", VC1, (float)VC1 / 4096.0 * 3.3 / 4.0);

            sleep_ms(2000);
        }
        */
        read_voltage(&VL1, 1, VL_SENSE);
        read_voltage(&VC1, 1, VC_SENSE);
        I1_avg = I1_avg * 0.99 + (float)VC1 / 4096.0 * 3.3 / 4.0 * 0.01;
            // moving average filter, bin=100
        if(I1_avg > target + margin) {
            if(N1 == 0) N1 = 0;
            else N1 = N1 - 1;
        } else if (I1_avg < target - margin){
            if(N1 == 255) N1 = 255;
            else N1 = N1 + 1;
        }
        RDAC_set(SPI_CS1, N1);
        busy_wait_us(5);
        x++;
        if(x % 10000 == 0) printf("N1: %u, VL1: %fV, VC1: %u, I1_avg: %fA\n", N1, (float)VL1 / 4096.0 * 3.3, VC1, I1_avg);
    }

    return 0;
}

inline static void RDAC_set(uint CS_pin, uint8_t val) {
    gpio_put(CS_pin, 0);
    spi_write_blocking(spi0, &val, 1);
    gpio_put(CS_pin, 1);
}

inline static void MUX_select(uint channel) {
    gpio_put_masked(MUX_mask, channel << MUX_S0);
}

inline static void read_voltage(uint16_t *voltage, uint channel, uint type) {
    if(type == VC_SENSE) {
        gpio_put_masked(MUX_mask, channel << MUX_S0);     // select MUX channel
        adc_select_input(0);                                // select ADC channel
        busy_wait_us(10);        // wait a little bit for MUX to settle
        *voltage = adc_read();
    } else if(type == VL_SENSE) {
        gpio_put_masked(MUX_mask, channel << MUX_S0);     // select MUX channel
        adc_select_input(1);                                // select ADC channel
        busy_wait_us(10);        // wait a little bit for MUX to settle
        *voltage = adc_read();
    }
}
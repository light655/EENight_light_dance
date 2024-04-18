#ifndef HELPER_H
#define HELPER_H

#include "hardware/adc.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"

#include "pindef.h"

uint SPI_CS_mask, PWM_mask, LDO_EN_mask, MUX_mask;

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

inline static void VC_read(uint16_t *to_store, uint channel) {
    gpio_put_masked(MUX_mask, channel << MUX_S0);       // select MUX channel
    adc_select_input(0);                                // select ADC channel
    busy_wait_us(5);                   // wait a little bit for MUX to settle
    *to_store = adc_read();
}

inline static void VL_read(uint16_t *to_store, uint channel) {
    gpio_put_masked(MUX_mask, channel << MUX_S0);       // select MUX channel
    adc_select_input(1);                                // select ADC channel
    busy_wait_us(5);                   // wait a little bit for MUX to settle
    *to_store = adc_read();
}

#endif
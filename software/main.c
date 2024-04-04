#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"

#define LDO_EN0 1
#define SPI_CS0 10
#define SPI_CLK 18
#define SPI_TX 19

void RDAC_set(uint CS_pin, uint8_t val);

int main(void) {
    stdio_init_all();

    spi_init(spi0, 100 * 1000);
    spi_set_format(spi0, 8, 0, 0, SPI_MSB_FIRST);
        // 8-bit, spi mode 0
    gpio_set_function(SPI_CLK, GPIO_FUNC_SPI);
    gpio_set_function(SPI_TX, GPIO_FUNC_SPI);
    gpio_init(SPI_CS0);
    gpio_set_dir(SPI_CS0, GPIO_OUT);
    gpio_put(SPI_CS0, 1);

    gpio_init(LDO_EN0);
    gpio_set_dir(LDO_EN0, GPIO_OUT);
    gpio_put(LDO_EN0, 1);

    while(true) {
        for(uint8_t i = 0; i <= 0xff; i += 16) {
            printf("RDAC wiper: %d\n", i);
            RDAC_set(SPI_CS0, i);
            sleep_ms(3000);
        }
    }

    return 0;
}

void RDAC_set(uint CS_pin, uint8_t val) {
    gpio_put(CS_pin, 0);
    spi_write_blocking(spi0, &val, 1);
    gpio_put(CS_pin, 1);

    return;
}
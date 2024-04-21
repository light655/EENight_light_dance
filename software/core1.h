#ifndef CORE1_H
#define CORE1_H

#include <stdio.h>
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/sync.h"

#include "midi_array.h"
#include "helper.h"
#include "pindef.h"

// main function to run on core 1
void core1_entry(void) {
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

    // LED initialisation -----------------------------------------------------
    uint LED_mask = (1 << LED_G) | (1 << LED_R) | (1 << LED_PICO);
    gpio_init_mask(LED_mask);
    gpio_set_dir_out_masked(LED_mask);
    gpio_put(LED_G, 1);

    // time synce pin initialisation ------------------------------------------
    gpio_init(T_SYNC);
    gpio_set_dir(T_SYNC, GPIO_IN);
    gpio_pull_up(T_SYNC);

    // Multicore initialisation -----------------------------------------------
    mutex_init(&intensity_mutex);

    // main code --------------------------------------------------------------
    gpio_put(PWM_PIN0, 1);
    gpio_put(PWM_PIN1, 1);
    gpio_put(PWM_PIN2, 1);
    gpio_put(PWM_PIN3, 1);

    uint64_t t_abs, t0, t;      // timing variables
    float I_target[4];          // target current for the 4 drivers
    int iAD = 0, iBD = 0, iCD = 0, iDD = 0; // indices for diminuendo
    int iA = 0, iB = 0, iC = 0, iD = 0;     // indices for normal lighting
    int iAC = 0, iBC = 0, iCC = 0, iDC = 0; // indices for crescendo
/*
    while(gpio_get(T_SYNC)) {   // spin around until T_SYNC line is grounded
        tight_loop_contents();
    }
    t0 = to_us_since_boot(get_absolute_time());     // time reference t = 0

    while(true) {
        t_abs = to_us_since_boot(get_absolute_time());
        t = t_abs - t0;


        // only update value if mutex is not taken
        if(mutex_try_enter(&intensity_mutex, NULL)) {
            intensities[0] = I_target[0];
            intensities[1] = I_target[1];
            intensities[2] = I_target[2];
            intensities[3] = I_target[3];
        }
    }
*/
    while(true) {
        I_target[1] = 0.0f;
        for(int i = 0; i < 256; i++) {
            I_target[1] += 0.00125;
            I_target[0] = I_target[1];
            I_target[2] = I_target[1];
            I_target[3] = I_target[1];
            busy_wait_us(11700);
            if(mutex_try_enter(&intensity_mutex, NULL)) {
                // printf("Core 1 entered mutex, sending %fA.\n", I_target[1]);
                intensities[0] = I_target[0];
                intensities[1] = I_target[1];
                intensities[2] = I_target[2];
                intensities[3] = I_target[3];
                mutex_exit(&intensity_mutex);
            }
        }
        sleep_ms(3000);

        for(int i = 0; i < 256; i++) {
            I_target[1] -= 0.00125;
            I_target[0] = I_target[1];
            I_target[2] = I_target[1];
            I_target[3] = I_target[1];
            busy_wait_us(11500);
            if(mutex_try_enter(&intensity_mutex, NULL)) {
                // printf("Core 1 entered mutex, sending %fA.\n", I_target[1]);
                intensities[0] = I_target[0];
                intensities[1] = I_target[1];
                intensities[2] = I_target[2];
                intensities[3] = I_target[3];
                mutex_exit(&intensity_mutex);
            }
        }
        sleep_ms(3000);
    }

    return;
}

#endif
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
    gpio_put(LED_PICO, 1);

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
    float I_target[4] = { 0.0f , 0.0f , 0.0f , 0.0f };          // target current for the 4 drivers
    int iAD = 0, iBD = 0, iCD = 0, iDD = 0; // indices for diminuendo
    int iA = 0, iB = 0, iC = 0, iD = 0;     // indices for normal lighting
    int iAC = 0, iBC = 0, iCC = 0, iDC = 0; // indices for crescendo
	
    while(gpio_get(T_SYNC)) {   // spin around until T_SYNC line is grounded
        tight_loop_contents();
    }
	
    t0 = to_us_since_boot(get_absolute_time());     // time reference t = 0
	gpio_put(LED_PICO, 0);

    while(true) {
        t_abs = to_us_since_boot(get_absolute_time());
        t = t_abs - t0;
		
		// deal with nomal lighting
		if ( A_t[iA] != (-1) ) {
			if ( t > A_t[iA] ) {
				I_target[0] = ((A_s[iA]/(float)127.0)*current_max[0]);
				iA ++;
			}
		}
		if ( B_t[iB] != (-1) ) {
			if ( t > B_t[iB] ) {
				I_target[2] = ((B_s[iB]/(float)127.0)*current_max[1]);
				iB ++;
			}
		}
		if ( C_t[iC] != (-1) ) {
			if ( t > C_t[iC] ) {
				I_target[3] = ((C_s[iC]/(float)127.0)*current_max[3]);
				iC ++;
			}
		}
		if ( D_t[iD] != (-1) ) {
			if ( t > D_t[iD] ) {
				I_target[1] = ((D_s[iD]/(float)127.0)*current_max[2]);
				iD ++;
			}
		}
		
		// deal with dim
		if ( AD_t[iAD] != (-1) ) {
			if ( t > AD_t[iAD+1] )
					iAD += 2;
			if ( t > AD_t[iAD] ) {
				I_target[0] = ( ( AD_t[iAD+1] - t )/(float)( AD_t[iAD+1] - AD_t[iAD] )* 
								(AD_s[iAD] / 127.0f) * current_max[0]  );
				}
			}
		if ( BD_t[iBD] != (-1) ) {
			if ( t > BD_t[iBD+1] )
					iBD += 2;
			if ( t > BD_t[iBD] ) {
				I_target[2] = ( ( BD_t[iBD+1] - t )/(float)( BD_t[iBD+1] - BD_t[iBD] )* 
								(BD_s[iBD] / 127.0f ) * current_max[1]  );
				}
			}
		if ( CD_t[iCD] != (-1) ) {
			if ( t > CD_t[iCD+1] )
					iCD += 2;
			if ( t > CD_t[iCD] ) {
				I_target[3] = ( ( CD_t[iCD+1] - t )/(float)( CD_t[iCD+1] - CD_t[iCD] )*
					   			( CD_s[iCD] / 127.0f ) * current_max[3]  );
				}
			}
		if ( DD_t[iDD] != (-1) ) {
			if ( t > DD_t[iDD+1] )
					iDD += 2;
			if ( t > DD_t[iDD] ) {
				I_target[1] = ( ( DD_t[iDD+1] - t )/(float)( DD_t[iDD+1] - DD_t[iDD] )*
					   			(DD_s[iDD] / 127.0f ) * current_max[2]  );
				}
			}

		// deal with cre
		if ( AC_t[iAC] != (-1) ) {
			if ( t > AC_t[iAC+1] )
					iAC += 2;
			if ( t > AC_t[iAC] ) {
				I_target[0] = ( ( t - AC_t[iAC] )/(float)( AC_t[iAC+1] - AC_t[iAC] )* 
								( AC_s[iAC] / 127.0f ) * current_max[0]  );
				}
			}
		if ( BC_t[iBC] != (-1) ) {
			if ( t > BC_t[iBC+1] )
					iBC += 2;
			if ( t > BC_t[iBC] ) {
				I_target[2] = ( ( t - BC_t[iBC] )/(float)( BC_t[iBC+1] - BC_t[iBC] )* 
								(BC_s[iBC] / 127.0f ) * current_max[1]  );
				}
			}
		if ( CC_t[iCC] != (-1) ) {
			if ( t > CC_t[iCC+1] )
					iCC += 2;
			if ( t > CC_t[iCC] ) {
				I_target[3] = ( ( t - CC_t[iCC] )/(float)( CC_t[iCC+1] - CC_t[iCC] )* 
								( CC_s[iCC] / 127.0f ) * current_max[3]  );
				}
			}
		if ( DC_t[iDC] != (-1) ) {
			if ( t > DC_t[iDC+1] )
					iDC += 2;
			if ( t > DC_t[iDC] ) {
				I_target[1] = ( ( t - DC_t[iDC] )/(float)( DC_t[iDC+1] - DC_t[iDC] )* 
								(DC_s[iDC] / 127.0f ) * current_max[2]  );
				}
			}
        
		// only update value if mutex is not taken
        if(mutex_try_enter(&intensity_mutex, NULL)) {
            intensities[0] = I_target[0];
            intensities[1] = I_target[1];
            intensities[2] = I_target[2];
            intensities[3] = I_target[3];
            mutex_exit(&intensity_mutex);
        }
    }
    return;
}

#endif

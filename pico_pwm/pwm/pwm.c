#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"

int main() {
	gpio_set_function( 14 , GPIO_FUNC_PWM );
	gpio_set_function( 15 , GPIO_FUNC_PWM );

	uint SN = pwm_gpio_to_slice_num(14);
	uint PC = pwm_gpio_to_channel(14);
	uint SN0 = pwm_gpio_to_slice_num(15);
	uint PC0 = pwm_gpio_to_channel(15);

	pwm_set_clkdiv(SN , 1);
	pwm_set_phase_correct(SN, 1 );
	pwm_set_wrap(SN , 127);
	pwm_set_clkdiv(SN0 , 1);
	pwm_set_phase_correct(SN0, 1 );
	pwm_set_wrap(SN0 , 127);

	pwm_set_chan_level(SN , PC , 64);
	pwm_set_chan_level(SN0 , PC , 64);
	pwm_set_counter (SN0 , 127)
	pwm_set_enabled(SN , 1);
	pwm_set_enabled(SN0 , 1);

}

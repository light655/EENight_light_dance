cmake_minimum_required(VERSION 3.13)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

include(pico_sdk_import.cmake)
project(blink C CXX ASM)

pico_sdk_init()
add_executable(pwm
        pwm.c
        )

# pull in common dependencies
target_link_libraries(pwm pico_stdlib hardware_pwm)

# create map/bin/hex file etc.
pico_add_extra_outputs(pwm)

# add url via pico_set_program_url
# example_auto_set_url(blink)

pico_enable_stdio_usb(pwm 1)
pico_enable_stdio_uart(pwm 1)

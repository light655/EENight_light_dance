cmake_minimum_required(VERSION 3.13)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

include(pico_sdk_import.cmake)
project(EENLD VERSION 0.1)
pico_sdk_init()

add_executable(EENLD
    main_all_current.c
    # main.c
)

add_compile_options()

target_link_libraries(EENLD
    hardware_adc
    hardware_gpio
    hardware_pwm
    hardware_spi
    pico_multicore
    pico_stdlib
    pico_sync
)

pico_add_extra_outputs(EENLD)
pico_enable_stdio_usb(EENLD 0)
pico_enable_stdio_uart(EENLD 1)
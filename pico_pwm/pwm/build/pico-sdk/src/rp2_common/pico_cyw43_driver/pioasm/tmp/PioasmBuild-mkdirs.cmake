# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/lisa/programs/binary/pico/pico-sdk/tools/pioasm"
  "/home/lisa/programs/binary/pico/Projects/pwm/build/pioasm"
  "/home/lisa/programs/binary/pico/Projects/pwm/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm"
  "/home/lisa/programs/binary/pico/Projects/pwm/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/tmp"
  "/home/lisa/programs/binary/pico/Projects/pwm/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src/PioasmBuild-stamp"
  "/home/lisa/programs/binary/pico/Projects/pwm/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src"
  "/home/lisa/programs/binary/pico/Projects/pwm/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src/PioasmBuild-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/lisa/programs/binary/pico/Projects/pwm/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src/PioasmBuild-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/lisa/programs/binary/pico/Projects/pwm/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src/PioasmBuild-stamp${cfgdir}") # cfgdir has leading slash
endif()

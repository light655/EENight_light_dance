# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/light/pico/pico-sdk/tools/pioasm"
  "/home/light/EENight_light_dance/software/build/pioasm"
  "/home/light/EENight_light_dance/software/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm"
  "/home/light/EENight_light_dance/software/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/tmp"
  "/home/light/EENight_light_dance/software/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src/PioasmBuild-stamp"
  "/home/light/EENight_light_dance/software/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src"
  "/home/light/EENight_light_dance/software/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src/PioasmBuild-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/light/EENight_light_dance/software/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src/PioasmBuild-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/light/EENight_light_dance/software/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src/PioasmBuild-stamp${cfgdir}") # cfgdir has leading slash
endif()

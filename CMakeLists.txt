cmake_minimum_required(VERSION 3.12)

include(lib/FreeRTOS/FreeRTOS/Source/portable/ThirdParty/GCC/RP2040/pico_sdk_import.cmake)
include(lib/PicoSDK/pico_sdk_init.cmake)

project(pico_experiments VERSION 0.1.0 LANGUAGES C CXX)

set(PICO_SDK_PATH ${PROJECT_SOURCE_DIR}/lib/PicoSDK)
set(FREERTOS_KERNEL_PATH ${PROJECT_SOURCE_DIR}/lib/FreeRTOS/FreeRTOS/Source)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

set(PICO_BOARD "pico_w")

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        )

add_subdirectory(src)

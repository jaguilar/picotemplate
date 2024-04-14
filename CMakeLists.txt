cmake_minimum_required(VERSION 3.13)

add_subdirectory(config)

set(PICO_SDK_PATH ${CMAKE_CURRENT_LIST_DIR}/lib/PicoSDK)
set(FREERTOS_KERNEL_PATH ${CMAKE_CURRENT_LIST_DIR}/lib/FreeRTOS/FreeRTOS/Source)
set(FREERTOS_PORT GCC_RP2040 CACHE STRING "")

include(${CMAKE_CURRENT_LIST_DIR}/lib/PicoSDK/external/pico_sdk_import.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/lib/FreeRTOS/FreeRTOS/Source/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/secrets.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/lib/SysprogsProfilerFramework/CMakeLists.txt)

project(pico_experiments VERSION 0.1.0 LANGUAGES C CXX ASM)

set(PICO_BOARD "pico_w")
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)

pico_sdk_init()

add_subdirectory(lib/LiquidCrystal_I2C)

add_compile_options(
        -Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Werror=narrowing
        -fdata-sections -ffunction-sections
        )

add_link_options(-Wl,--gc-sections)

add_library(freertos INTERFACE)
target_link_libraries(freertos
        INTERFACE
        FreeRTOS-Kernel
        freertos_config
        FreeRTOS-Kernel-Heap4
        pico_sync
)

add_library(lwip INTERFACE)
target_link_libraries(lwip
        INTERFACE
        lwipopts pico_cyw43_arch_lwip_sys_freertos pico_lwip_api
        pico_lwip_core pico_lwip_freertos pico_lwip_mdns)

set(LWIP_DIR "${CMAKE_CURRENT_LIST_DIR}/lib/PicoSDK/lib/lwip")
add_subdirectory(lib/lwip-cpp)
add_subdirectory(src)

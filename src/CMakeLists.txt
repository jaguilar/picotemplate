include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(common STATIC shared_init.c FreeRTOSConfig.h)
target_link_libraries(common PUBLIC FreeRTOS-Kernel FreeRTOS-Kernel-Heap4 pico_cyw43_arch_lwip_sys_freertos pico_lwip_freertos pico_stdlib)
target_compile_options(common PUBLIC -fdata-sections -ffunction-sections)
target_link_options(common PUBLIC -Wl,--gc-sections -z max-page-size=4096)

add_executable(blink blink.c)
target_link_libraries(blink PRIVATE common)
pico_add_extra_outputs(blink)
pico_enable_stdio_usb(blink 1)
pico_enable_stdio_uart(blink 0)


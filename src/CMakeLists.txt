include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_executable(blink blink.c FreeRTOSConfig.h)

target_compile_options(blink PRIVATE -fdata-sections -ffunction-sections)
target_link_libraries(blink FreeRTOS-Kernel FreeRTOS-Kernel-Heap4 pico_cyw43_arch_none pico_stdlib)
target_link_options(blink PRIVATE -Wl,--gc-sections -z max-page-size=4096)
pico_add_extra_outputs(blink)
pico_enable_stdio_usb(blink 1)
pico_enable_stdio_uart(blink 0)
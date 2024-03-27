include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(common STATIC shared_init.c FreeRTOSConfig.h)
target_link_libraries(common PUBLIC 
    FreeRTOS-Kernel 
    FreeRTOS-Kernel-Heap4 
    pico_cyw43_arch_lwip_sys_freertos 
    pico_lwip_api 
    pico_lwip_core 
    pico_lwip_freertos 
    pico_lwip_mdns 
    pico_stdlib 
    pico_sync
)
target_compile_options(
    common 
    PUBLIC -fdata-sections -ffunction-sections 
    PRIVATE -DWIFI_PASSWORD="${WIFI_PASSWORD}" -DWIFI_SSID="${WIFI_SSID}")

target_link_options(common PUBLIC -Wl,--gc-sections -z max-page-size=4096)

add_executable(blink blink.c)
target_link_libraries(blink PRIVATE common)
pico_add_extra_outputs(blink)
pico_enable_stdio_usb(blink 1)
pico_enable_stdio_uart(blink 0)

add_executable(tcp_echo tcp_echo.c)
target_link_libraries(tcp_echo PRIVATE common)
pico_add_extra_outputs(tcp_echo)
pico_enable_stdio_usb(tcp_echo 1)
pico_enable_stdio_uart(tcp_echo 0)


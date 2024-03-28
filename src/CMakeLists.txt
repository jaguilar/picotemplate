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
    PRIVATE -DWIFI_PASSWORD="${WIFI_PASSWORD}" -DWIFI_SSID="${WIFI_SSID}" -DCYW43_HOST_NAME="picotest")

target_link_options(common PUBLIC -Wl,--gc-sections -z max-page-size=4096)

add_subdirectory(blink)
add_subdirectory(tcp_echo)
add_subdirectory(hello_mqtt)
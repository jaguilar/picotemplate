include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(common STATIC shared_init.c shared_init.h)
target_link_libraries(common PUBLIC 
    freertos
    lwip
    pico_stdlib
    pico_sync
)
target_compile_options(
    common 
    PRIVATE -DWIFI_PASSWORD="${WIFI_PASSWORD}" -DWIFI_SSID="${WIFI_SSID}" -DCYW43_HOST_NAME="picotest")

function(pico_executable name)
  add_executable(${name} ${ARGN})
  pico_add_extra_outputs(${name})
  pico_enable_stdio_usb(${name} 1)
  pico_enable_stdio_uart(${name} 0)
endfunction()

add_subdirectory(blink)
add_subdirectory(tcp_echo)
add_subdirectory(hello_mqtt)
add_subdirectory(sun)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_C_STANDARD_REQUIRED 11)
set(CMAKE_CXX_STANDARD_REQUIRED 23)

add_library(common STATIC shared_init.c shared_init.h)
target_link_libraries(common PUBLIC 
    freertos
    lwip
    pico_stdlib
    pico_sync
    stdc++
)
target_compile_options(
    common 
    PRIVATE -DWIFI_PASSWORD="${WIFI_PASSWORD}" -DWIFI_SSID="${WIFI_SSID}" -DCYW43_HOST_NAME="picotest")

add_library(common_nonet STATIC shared_init.c shared_init.h)
target_link_libraries(common_nonet PUBLIC 
    freertos
    pico_stdlib
    pico_sync
    pico_cyw43_arch_none
    stdc++
)


function(add_pico_executable name)
  add_executable(${name} ${ARGN})
  pico_add_extra_outputs(${name})
  pico_enable_stdio_usb(${name} 0)
  pico_enable_stdio_uart(${name} 1)
endfunction()

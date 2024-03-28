add_executable(tcp_echo tcp_echo.c)
target_link_libraries(tcp_echo PRIVATE common)
pico_add_extra_outputs(tcp_echo)
pico_enable_stdio_usb(tcp_echo 1)
pico_enable_stdio_uart(tcp_echo 0)


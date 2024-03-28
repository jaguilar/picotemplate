add_executable(hello_mqtt hello_mqtt.c)
target_compile_options(
  hello_mqtt 
  PRIVATE -DMQTT_HOST="rpi5.local" -DMQTT_USER="${MQTT_USER}" -DMQTT_PASSWORD="${MQTT_PASSWORD}")
target_link_libraries(hello_mqtt PRIVATE common pico_lwip_mqtt)
pico_add_extra_outputs(hello_mqtt)
pico_enable_stdio_usb(hello_mqtt 1)
pico_enable_stdio_uart(hello_mqtt 0)


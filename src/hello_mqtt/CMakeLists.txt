add_pico_executable(hello_mqtt hello_mqtt.cc)
target_compile_options(
  hello_mqtt 
  PRIVATE -DMQTT_HOST="rpi5.local" -DMQTT_USER="${MQTT_USER}" -DMQTT_PASSWORD="${MQTT_PASSWORD}")
target_link_libraries(hello_mqtt PRIVATE common pico_lwip_mqtt stdc++)
set_target_properties(hello_mqtt PROPERTIES 
    CXX_STANDARD_REQUIRED ON
)


add_pico_executable(exec_2_1 src_2_1.cc)
target_link_libraries(exec_2_1 common_nonet)

add_pico_executable(exec_2_2 src_2_2.cc)
target_link_libraries(exec_2_2 common_nonet)
target_compile_features(exec_2_2 PUBLIC cxx_std_23)

add_pico_executable(exec_2_3 src_2_3.cc)
target_link_libraries(exec_2_3 common_nonet hardware_pwm)
target_compile_features(exec_2_3 PUBLIC cxx_std_23)

add_pico_executable(exec_2_4 src_2_4.cc)
target_link_libraries(exec_2_4 common_nonet hardware_pwm)
target_compile_features(exec_2_4 PUBLIC cxx_std_23)

add_pico_executable(exec_2_5 src_2_5.cc)
target_link_libraries(exec_2_5 common_nonet hardware_gpio)
target_compile_features(exec_2_5 PUBLIC cxx_std_23)

add_pico_executable(exec_3_4 src_3_4.cc)
target_link_libraries(exec_3_4 PRIVATE common_nonet LiquidCrystal_I2C hardware_i2c)
target_compile_features(exec_3_4 PRIVATE cxx_std_23)

add_library(driver_cd74hc595 cd74hc595.cc cd74hc595.h)
target_link_libraries(driver_cd74hc595 PUBLIC hardware_gpio hardware_pio PRIVATE common_nonet hardware_pio)
pico_generate_pio_header(driver_cd74hc595 ${CMAKE_CURRENT_LIST_DIR}/cd74hc595.pio)

add_pico_executable(exec_5_1 src_5_1.cc)
target_link_libraries(exec_5_1 common_nonet hardware_gpio hardware_pio driver_cd74hc595)
target_compile_features(exec_5_1 PUBLIC cxx_std_23)

add_library(disp_4digit disp_4digit.h disp_4digit.cc)
target_compile_options(disp_4digit PRIVATE -fstack-usage)
target_link_libraries(disp_4digit PUBLIC hardware_gpio driver_cd74hc595 freertos)

add_pico_executable(exec_5_3 src_5_3.cc)
target_link_libraries(exec_5_3 common_nonet hardware_pio disp_4digit driver_cd74hc595)

add_pico_executable(exec_5_4 src_5_4.cc)
target_link_libraries(exec_5_4 common_nonet hardware_gpio hardware_pio driver_cd74hc595 hardware_timer)
target_compile_features(exec_5_4 PUBLIC cxx_std_23)

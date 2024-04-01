add_pico_executable(exec_2_1 src_2_1.cc)
target_link_libraries(exec_2_1 common_nonet)

add_pico_executable(exec_2_2 src_2_2.cc)
target_link_libraries(exec_2_2 common_nonet)
target_compile_features(exec_2_2 PUBLIC cxx_std_23)

add_pico_executable(exec_2_3 src_2_3.cc)
target_link_libraries(exec_2_3 common_nonet hardware_pwm)
target_compile_features(exec_2_3 PUBLIC cxx_std_23)

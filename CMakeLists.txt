cmake_minimum_required(VERSION 3.0.0)

include(${CMAKE_CURRENT_SOURCE_DIR}/PicoSDK/pico_sdk_init.cmake)

project(pico_experiments VERSION 0.1.0 LANGUAGES C CXX)

pico_sdk_init()

include(CTest)
enable_testing()



add_executable(pico_experiments main.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

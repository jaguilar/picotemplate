set(CXX_STANDARD_REQUIRED 23)

add_library(freertos_config INTERFACE FreeRTOSConfig.h)
target_include_directories(freertos_config SYSTEM INTERFACE ${CMAKE_CURRENT_LIST_DIR}) 
target_compile_definitions(freertos_config INTERFACE projCOVERAGE_TEST=0)

add_library(lwipopts INTERFACE lwipopts.h )
target_include_directories(lwipopts SYSTEM INTERFACE ${CMAKE_CURRENT_LIST_DIR}) 

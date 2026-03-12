add_library(service_lib STATIC 
    DeviceService.cpp
    DeviceService.h
)

target_include_directories(service_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(service_lib PUBLIC domain_lib scanner_lib)
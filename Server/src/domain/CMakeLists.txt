add_library(domain_lib STATIC 
    DeviceManager.cpp
    DeviceManager.h
    Device.h
    NetworkInfo.h
)

target_include_directories(domain_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

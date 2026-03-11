add_library(api_lib STATIC HttpSrv.cpp routing/DeviceRoutes.cpp)

target_include_directories(api_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(api_lib PUBLIC httplib PRIVATE nlohmann_json service_lib)
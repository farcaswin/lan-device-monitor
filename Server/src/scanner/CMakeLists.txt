add_library(scanner_lib STATIC 
    NetworkScanner.cpp
    NetworkScanner.h
)

target_include_directories(scanner_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(scanner_lib PUBLIC domain_lib)
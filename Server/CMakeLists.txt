cmake_minimum_required(VERSION 3.20)
project(FuzzerServer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_library(httplib INTERFACE)
target_include_directories(httplib INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/external/httplib
)

add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/external/json
)

add_subdirectory(src/domain)
add_subdirectory(src/scanner)
add_subdirectory(src/services)
add_subdirectory(src/api)

add_executable(fuzzer-srv main.cpp)

target_link_libraries(fuzzer-srv PRIVATE api_lib service_lib domain_lib scanner_lib)
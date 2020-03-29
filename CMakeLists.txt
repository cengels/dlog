cmake_minimum_required(VERSION 3.10)
project(dlog LANGUAGES CXX VERSION 0.0.1)
add_executable(dlog main.cpp)
configure_file(version.h.in version.h)

if (NOT EXISTS "${CMAKE_SOURCE_DIR}/libs/cxxopts.hpp")
    file(DOWNLOAD "https://raw.githubusercontent.com/jarro2783/cxxopts/master/include/cxxopts.hpp" "${CMAKE_SOURCE_DIR}/libs/cxxopts.hpp")
endif()

include_directories(dlog PUBLIC "${PROJECT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/libs")
target_include_directories(dlog PUBLIC "${PROJECT_BINARY_DIR}" "${PROJECT_BINARY_DIR}/libs")
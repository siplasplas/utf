cmake_minimum_required(VERSION 3.13)
project(utf VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_definitions("-Werror=return-type")

# Include directories
include_directories(include)

# Tests
add_executable(gtests
        test/test.cpp)

target_link_libraries(gtests gtest gtest_main)

# Converter tool
add_executable(convutf converter.cpp
        Args.h
        Args.cpp)

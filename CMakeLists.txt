cmake_minimum_required(VERSION 3.13)
project(utf VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_definitions("-Werror=return-type")

add_executable(gtests
        test.cpp)

target_link_libraries(gtests gtest gtest_main)

add_executable(convutf converter.cpp
        Args.h
        Args.cpp)

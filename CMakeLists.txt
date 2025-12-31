cmake_minimum_required(VERSION 3.13)
project(utf VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# UTF library (static)
add_library(utf STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/generated/CaseData.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/generated/DecompData.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/generated/CollationData.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Collator.cpp)

# Include directory - works both standalone and as submodule
target_include_directories(utf PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

# Alias for consistent usage: target_link_libraries(myapp utf::utf)
add_library(utf::utf ALIAS utf)

# C++17 required
target_compile_features(utf PUBLIC cxx_std_17)

# Only build tests and tools when this is the main project
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    add_definitions("-Werror=return-type")

    # Tests
    add_executable(gtests
            test/test.cpp)
    target_link_libraries(gtests utf gtest gtest_main)

    # Converter tool
    add_executable(convutf converter.cpp
            Args.h
            Args.cpp)
    target_link_libraries(convutf utf)
endif()

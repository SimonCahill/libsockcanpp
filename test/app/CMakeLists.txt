cmake_minimum_required(VERSION 3.14)

project(testlibsockcanpp LANGUAGES CXX VERSION 1.0.0)
set(TARGET_NAME testsockcanpp.bin)

set(CMAKE_CXX_STANDARD 14)

include_directories(
    include/
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_BINARY_DIR}/libsockcanpp)

file(GLOB_RECURSE FILES ${CMAKE_CURRENT_SOURCE_DIR} src/*.cpp)

add_executable(${TARGET_NAME} ${FILES})

target_link_libraries(
    # Binary
    ${TARGET_NAME}

    # Libs
    sockcanpp
    -lm

    -static
    -static-libstdc++
    -static-libgcc
)
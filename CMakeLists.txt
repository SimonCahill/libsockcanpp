cmake_minimum_required(VERSION 3.10)

project(sockcanpp LANGUAGES CXX VERSION 1.00)
set(TARGET_NAME sockcanpp)
set(CMAKE_CXX_STANDARD 11)

include_directories(
    include/
)

file(GLOB_RECURSE FILES ${CMAKE_CURRENT_SOURCE_DIR} src/*.cpp)

add_library(${TARGET_NAME} STATIC ${FILES})

target_include_directories(
    ${TARGET_NAME}

    INTERFACE include/
    INTERFACE include/exceptions
)


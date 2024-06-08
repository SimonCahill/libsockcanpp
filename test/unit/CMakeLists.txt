cmake_minimum_required(VERSION 3.10)

project(libsockcanpp_unittests LANGUAGES CXX VERSION 0.1)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GTest REQUIRED)

include_directories(
    # none as of now
)

if (NOT TARGET sockcanpp)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../ ${CMAKE_CURRENT_BINARY_DIR}/libsockcanpp)
endif()

file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Werror
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(
    ${PROJECT_NAME}

    sockcanpp
    ${GTEST_LIBRARIES}
    pthread
)
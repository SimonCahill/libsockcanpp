cmake_minimum_required(VERSION 3.14)

project(testlibsockcanpp_can_fd LANGUAGES CXX VERSION 1.0.0)
set(TARGET_NAME testsockcanpp_can_fd.bin)

set(CMAKE_CXX_STANDARD 14)

include_directories(
    include/
)

if (NOT TARGET sockcanpp)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../.. ${CMAKE_CURRENT_BINARY_DIR}/libsockcanpp)
endif()

file(GLOB_RECURSE FILES ${CMAKE_CURRENT_SOURCE_DIR} src/*.cpp)

add_executable(${TARGET_NAME} ${FILES})

if (TARGET sockcanpp_test)
    set(SOCKCANPP_LINK_TARGET sockcanpp_test)
    set(SOCKCANPP_LINK_FLAGS -static -static-libstdc++ -static-libgcc)
else()
    set(SOCKCANPP_LINK_TARGET sockcanpp)
    set(SOCKCANPP_LINK_FLAGS)
endif()

target_link_libraries(
    ${TARGET_NAME}

    ${SOCKCANPP_LINK_TARGET}
    -lm

    ${SOCKCANPP_LINK_FLAGS}
)

cmake_minimum_required(VERSION 3.14)

project(testlibsockcanpp LANGUAGES CXX VERSION 1.0.0)
set(TARGET_NAME testsockcanpp.bin)

file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

foreach(SOURCE ${SOURCES})
    get_filename_component(TEST_NAME ${SOURCE} NAME_WE)
    add_executable(${TEST_NAME} ${SOURCE})
    target_link_libraries(${TEST_NAME} PRIVATE sockcanpp_test sockcanpp::libmnl)
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()

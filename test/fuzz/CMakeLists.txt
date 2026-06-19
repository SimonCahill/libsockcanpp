cmake_minimum_required(VERSION 3.16)

if (NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR
        "BUILD_FUZZ_TESTS requires Clang with libFuzzer support; "
        "CMake is currently using ${CMAKE_CXX_COMPILER_ID} (${CMAKE_CXX_COMPILER}). "
        "Reconfigure with -DCMAKE_CXX_COMPILER=clang++."
    )
endif()

add_executable(sockcanpp_message_fuzzer src/MessageFuzzer.cpp)

target_include_directories(sockcanpp_message_fuzzer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

if (TARGET sockcanpp_test)
    set(LIBSOCKCANPP_FUZZ_TARGET sockcanpp_test)
else()
    set(LIBSOCKCANPP_FUZZ_TARGET sockcanpp)
endif()

target_link_libraries(sockcanpp_message_fuzzer PRIVATE
    ${LIBSOCKCANPP_FUZZ_TARGET}
)

target_compile_options(sockcanpp_message_fuzzer PRIVATE
    -fsanitize=fuzzer,address,undefined
    -fno-omit-frame-pointer
)

target_link_options(sockcanpp_message_fuzzer PRIVATE
    -fsanitize=fuzzer,address,undefined
)

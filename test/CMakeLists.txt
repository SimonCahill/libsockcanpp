cmake_minimum_required(VERSION 3.12)

add_subdirectory(app)
add_subdirectory(unit)

if (BUILD_FUZZ_TESTS)
    add_subdirectory(fuzz)
endif()

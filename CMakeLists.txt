cmake_minimum_required(VERSION 3.23)

project(sockcanpp LANGUAGES CXX VERSION 1.8.0 DESCRIPTION "A simple C++ wrapper for the Linux SocketCAN API" HOMEPAGE_URL "https://github.com/simoncahill/libsockcanpp")

option(BUILD_SHARED_LIBS "Build shared libraries (.so) instead of static ones (.a)" ON)
option(BUILD_TESTS "Build the tests" OFF)
option(sockcanpp_CANXL_SUPPORT "Enable CAN XL message and driver APIs" OFF)
option(sockcanpp_NO_INCL_DIR "Expose headers without the sockcanpp/ include prefix" OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(sockcanpp_CONCEPT_SUPPORT "Allow higher level language support" ON)

if (sockcanpp_CANXL_SUPPORT)
    include(CheckCXXSourceCompiles)
    check_cxx_source_compiles([=[
        #include <linux/can.h>
        #include <linux/can/raw.h>
        int main() {
            canxl_frame frame{};
            frame.flags = CANXL_XLF;
            return CAN_RAW_XL_FRAMES + static_cast<int>(CANXL_MTU);
        }
    ]=] sockcanpp_HAS_CANXL_KERNEL_HEADERS)

    if (NOT sockcanpp_HAS_CANXL_KERNEL_HEADERS)
        message(FATAL_ERROR "sockcanpp_CANXL_SUPPORT requires Linux kernel headers with CAN XL support")
    endif()
endif()

if (sockcanpp_CONCEPT_SUPPORT)
    message(STATUS "Enabling support for higher level language features")
    set(CMAKE_CXX_STANDARD 20)
else()
    set(CMAKE_CXX_STANDARD 11)
endif()

add_compile_options(
    -Wall
    -Werror
    -Wpedantic
    -Wno-unknown-pragmas
)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")

if (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_link_options($<$<CONFIG:RELEASE>:-s>)
endif()

include(GNUInstallDirs)

if (BUILD_SHARED_LIBS STREQUAL "ON")
    add_library(${PROJECT_NAME})
    set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${PROJECT_VERSION})
    set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
else()
    add_library(${PROJECT_NAME} STATIC)
endif()

###
# If BUILD_TESTS is set to ON, a static test library with the name of the project suffixed with "_test" will be created
###
if(BUILD_TESTS STREQUAL "ON")
    enable_testing()
    add_library(${PROJECT_NAME}_test STATIC)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test/)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
if (sockcanpp_CANXL_SUPPORT)
    target_compile_definitions(${PROJECT_NAME} PUBLIC SOCKCANPP_CANXL_SUPPORT=1)
endif()

if (TARGET ${PROJECT_NAME}_test)
    target_include_directories(${PROJECT_NAME}_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    if (sockcanpp_CANXL_SUPPORT)
        target_compile_definitions(${PROJECT_NAME}_test PUBLIC SOCKCANPP_CANXL_SUPPORT=1)
    endif()
endif()

add_subdirectory(include)
add_subdirectory(src)

install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
    FILE_SET HEADERS
)

install(EXPORT ${PROJECT_NAME}Targets
    FILE "lib${PROJECT_NAME}Targets.cmake"
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION lib/cmake/${PROJECT_NAME}
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "lib${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${${PROJECT_NAME}_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES "cmake/lib${PROJECT_NAME}Config.cmake" "${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION lib/cmake/${PROJECT_NAME})


set(prefix      ${CMAKE_INSTALL_PREFIX})
set(exec_prefix ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR})
set(includedir  ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR})
set(libdir      ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})

configure_file(libsockcanpp.pc.in ${CMAKE_BINARY_DIR}/libsockcanpp.pc @ONLY)
# Install pkg-config files
install(FILES ${CMAKE_BINARY_DIR}/libsockcanpp.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

include(cmake/cpack.cmake)

###
# Docs target
###
add_custom_target("libsockcan_docs" COMMENT "Create Doxygen documentation")
add_custom_command(
    TARGET "libsockcan_docs"
    POST_BUILD
        COMMENT "Generate Doxygen documentation for publication or reading"
        COMMAND doxygen ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

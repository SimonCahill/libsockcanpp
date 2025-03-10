##################################################
##  Simple script which fetches and configures  ##
##  the libmnl library for use in a CMake-based ##
##  project.                                    ##
##                                              ##
##  Written by Simon Cahill (contact@simonc.eu) ##
##  Copyright (c) 2025 Simon Cahill             ##
##################################################

include(ExternalProject)

# detect the total number of cores on the system
include(ProcessorCount)
ProcessorCount(NUM_CORES)
math(EXPR USABLE_CORES "${NUM_CORES} - 1")

ExternalProject_Add(
    libmnl-ext
    GIT_REPOSITORY git://git.netfilter.org/libmnl
    GIT_TAG master

    # Set the CMake options for the libmnl library
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/libmnl_source
    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/libmnl_install

    UPDATE_DISCONNECTED ON

    CONFIGURE_COMMAND
        cd ${CMAKE_CURRENT_BINARY_DIR}/libmnl_source/ && ${CMAKE_CURRENT_BINARY_DIR}/libmnl_source/autogen.sh && ${CMAKE_CURRENT_BINARY_DIR}/libmnl_source/configure --prefix=${CMAKE_CURRENT_BINARY_DIR}/libmnl_install --host=${CMAKE_HOST_SYSTEM_PROCESSOR}-linux --with-doxygen --enable-static

    INSTALL_COMMAND cd ${CMAKE_CURRENT_BINARY_DIR}/libmnl_source/ && make install

    BUILD_COMMAND cd ${CMAKE_CURRENT_BINARY_DIR}/libmnl_source/ && make -j${USABLE_CORES}
)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/libmnl_install/include)

add_library(libmnl STATIC IMPORTED)
set_target_properties(libmnl PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/libmnl_install/lib/libmnl.a)
set_target_properties(libmnl PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_BINARY_DIR}/libmnl_install/include)
set_target_properties(libmnl PROPERTIES INCLUDE_DIRECTORIES ${CMAKE_CURRENT_BINARY_DIR}/libmnl_install/include)

add_library(libmnl::libmnl ALIAS libmnl)
add_library(sockcanpp::libmnl ALIAS libmnl)

add_dependencies(libmnl libmnl-ext)
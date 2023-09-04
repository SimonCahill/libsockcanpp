cmake_minimum_required(VERSION 3.10)

project(sockcanpp LANGUAGES CXX VERSION 1.00)

option(BUILD_SHARED_LIBS "Build shared libraries (.dll/.so) instead of static ones (.lib/.a)" OFF)

set(CMAKE_CXX_STANDARD 11)

include_directories(
    include/
)

file(GLOB_RECURSE FILES ${CMAKE_CURRENT_SOURCE_DIR} src/*.cpp)

add_library(${PROJECT_NAME} ${FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${PROJECT_VERSION})

target_include_directories(
    ${PROJECT_NAME}

    INTERFACE include/
    INTERFACE include/exceptions
)

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}  DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})

set(prefix      ${CMAKE_INSTALL_PREFIX})
set(exec_prefix ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR})
set(includedir  ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR})
set(libdir      ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})

configure_file(libsockcanpp.pc.in ${CMAKE_BINARY_DIR}/libsockcanpp.pc @ONLY)
# Install pkg-config files
install(FILES ${CMAKE_BINARY_DIR}/libsockcanpp.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

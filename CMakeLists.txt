cmake_minimum_required(VERSION 3.14)

project(sockcanpp LANGUAGES CXX VERSION 1.1.0)

option(BUILD_SHARED_LIBS "Build shared libraries (.dll/.so) instead of static ones (.lib/.a)" ON)
option(BUILD_TESTS "Build the tests" OFF)

set(CMAKE_CXX_STANDARD 11)
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
    add_library(${PROJECT_NAME}_test STATIC)
endif()

add_subdirectory(include)
add_subdirectory(src)

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.23")
    install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}Targets
        FILE_SET HEADERS
    )

    install(EXPORT ${PROJECT_NAME}Targets
        FILE ${PROJECT_NAME}Targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION lib/cmake/${PROJECT_NAME}
    )
endif()

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
add_custom_target("docs" COMMENT "Create Doxygen documentation")
add_custom_command(
    TARGET "docs"
    POST_BUILD
        COMMENT "Generate Doxygen documentation for publication or reading"
        COMMAND doxygen ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

###
# If the CMAKE_BUILD_TYPE is set to Debug, enable the tests
###
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND BUILD_TESTS STREQUAL "ON")
    enable_testing()
    add_subdirectory(test)
endif()

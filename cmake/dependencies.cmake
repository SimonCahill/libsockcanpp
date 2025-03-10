##################################################################
##   This file ensures that the build system is aware of all    ##
##   dependencies that are required for the project to build.   ##
##                                                              ##
##  All submodules are included here.                           ##
##                                                              ##
##  Written by Simon Cahill (s.cahill@procyon-systems.de)       ##
##  Copyright (c) 2025 Procyon Systems                          ##
##################################################################

# Handle submodules

list(APPEND ${PROJECT_NAME}_SUBMODULES

    # list entries go here, paths are relative to the project root
)

foreach(SUBMODULE ${${PROJECT_NAME}_SUBMODULES})
    add_subdirectory(${SUBMODULE})
endforeach()

# Handle other CMake includes

# Add all files ending with .cmake in the cmake/cmake_includes directory to the list of CMake includes
file(GLOB ${PROJECT_NAME}_CMAKE_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_includes/*.cmake")
list(APPEND ${PROJECT_NAME}_CMAKE_INCLUDES

    # list entries go here, paths are relative to the project root
)

foreach(CMAKE_INCLUDE ${${PROJECT_NAME}_CMAKE_INCLUDES})
    include(${CMAKE_INCLUDE})
endforeach()

###
# Handle CPM dependencies
###
list(APPEND ${PROJECT_NAME}_CPM_PACKAGES
    ""
)

foreach(CPM_PACK ${${PROJECT_NAME}_CPM_PACKAGES})
    string(REPLACE "§" ";" PACKAGE_INFO ${CPM_PACK})
    list(GET PACKAGE_INFO 0 PACKAGE_NAME)
    list(GET PACKAGE_INFO 1 PACKAGE_REPO)
    list(GET PACKAGE_INFO 2 PACKAGE_TAG)
    
    message("STATUS" "Adding CPM package: ${PACKAGE_NAME} from ${PACKAGE_REPO} at tag ${PACKAGE_TAG}")

    CPMAddPackage(
        NAME ${PACKAGE_NAME}
        GIT_REPOSITORY ${PACKAGE_REPO}
        GIT_TAG ${PACKAGE_TAG}
    )

    list(APPEND <APP_NAME>_CPM_TARGETS ${PACKAGE_NAME})
endforeach()

unset(PACKAGE_INFO)
unset(PACKAGE_NAME)
unset(PACKAGE_REPO)
unset(PACKAGE_TAG)
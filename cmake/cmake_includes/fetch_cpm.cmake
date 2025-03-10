##################################################################
##  This script will dynamically download the latest version of ##
##  CPM from GitHub and will include it in the project.         ##
##                                                              ##
##  Written by Simon Cahill (s.cahill@procyon-systems.de)       ##
##  Copyright (c) 2025 Procyon Systems                          ##
##################################################################

set(${PROJECT_NAME}_CPM_DOWNLOAD_DIR "${CMAKE_CURRENT_BINARY_DIR}/cmake/cmake_includes")
file(MAKE_DIRECTORY "${PROJECT_NAME}")

# Download CPM
file(DOWNLOAD
    "https://github.com/cpm-cmake/CPM.cmake/releases/latest/download/CPM.cmake"
    "${${PROJECT_NAME}_CPM_DOWNLOAD_DIR}/CPM.cmake"
)

# Include CPM
include("${${PROJECT_NAME}_CPM_DOWNLOAD_DIR}/CPM.cmake")
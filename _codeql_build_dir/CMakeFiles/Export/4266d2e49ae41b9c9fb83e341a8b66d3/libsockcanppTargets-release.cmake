#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "sockcanpp::sockcanpp" for configuration "Release"
set_property(TARGET sockcanpp::sockcanpp APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(sockcanpp::sockcanpp PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/sockcanpp.so.1.7.4"
  IMPORTED_SONAME_RELEASE "sockcanpp.so.1.7.4"
  )

list(APPEND _cmake_import_check_targets sockcanpp::sockcanpp )
list(APPEND _cmake_import_check_files_for_sockcanpp::sockcanpp "${_IMPORT_PREFIX}/lib/sockcanpp.so.1.7.4" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)

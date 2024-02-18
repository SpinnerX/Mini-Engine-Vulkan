#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Mini-GameEngine::Mini-GameEngine" for configuration "Debug"
set_property(TARGET Mini-GameEngine::Mini-GameEngine APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(Mini-GameEngine::Mini-GameEngine PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libMini-GameEngine.a"
  )

list(APPEND _cmake_import_check_targets Mini-GameEngine::Mini-GameEngine )
list(APPEND _cmake_import_check_files_for_Mini-GameEngine::Mini-GameEngine "${_IMPORT_PREFIX}/lib/libMini-GameEngine.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)

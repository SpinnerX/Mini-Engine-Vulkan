cmake_minimum_required(VERSION 3.15)

set(PROJECT_NAME Mini-GameEngine)
project(${PROJECT_NAME} VERSION 1.0)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "") # works (in creating the compile_commands.json file)

option(GAMEENGINE_BUILD_UNIT_TESTS "Build the GameEngine unit tests" ON)
option(GAMEENGINE_BUILD_TESTBED "Build the GameEngine testbed" ON)
option(GAMEENGINE_BUILD_DOCS "Build the GameEngine documentation" OFF)
option(GAMEENGINE_USER_SETTINGS "Override GameEngine settings with GameEngineUserSettings.h" OFF)

option(BUILD_SHARED_LIBS "Build Box2D as a shared library" OFF)

include(GNUInstallDirs)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(src)

message("Message = ${CMAKE_INSTALL_PREFIX}")

target_precompile_headers(${PROJECT_NAME} PUBLIC
	"$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_INSTALL_PREFIX}/include/GameEngine/GameEnginePrecompiledHeader.h>"
)

install(
  DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/GameEngine"
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Installing to /usr/local/public/GameEngine/
# install(
#   DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets"
#   DESTINATION "${CMAKE_INSTALL_LIBDIR}/GameEngine"
# )

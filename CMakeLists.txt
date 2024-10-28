cmake_minimum_required(VERSION 3.15)

set(PROJECT_NAME Mini-GameEngine)
project(${PROJECT_NAME} VERSION 1.0)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "") # works (in creating the compile_commands.json file)

option(GAMEENGINE_BUILD_UNIT_TESTS "Build the GameEngine unit tests" ON)
option(ENGINE_BUILD_SANDBOX "Building Min-Engine Sandbox" ON)
option(GAMEENGINE_BUILD_DOCS "Build the GameEngine documentation" OFF)
option(GAMEENGINE_USER_SETTINGS "Override GameEngine settings with GameEngineUserSettings.h" OFF)

option(BUILD_SHARED_LIBS "Build Box2D as a shared library" OFF)

find_package(Vulkan REQUIRED)


# find_dependency(OpenGL REQUIRED )

# include_directories( ${OPENGL_INCLUDE_DIRS} )

# set( GLFW_BUILD_DOCS OFF CACHE BOOL  "GLFW lib only" )
# set( GLFW_INSTALL OFF CACHE BOOL  "GLFW lib only" )
# set( GLAD_GL "" )
# add_subdirectory(external/glfw)

# option( GLFW-CMAKE-STARTER-USE-GLFW-GLAD "Use GLAD from GLFW" ON )

# if( GLFW-CMAKE-STARTER-USE-GLFW-GLAD )
#     include_directories("${GLFW_SOURCE_DIR}/deps")
#     set( GLAD_GL "${GLFW_SOURCE_DIR}/deps/glad/gl.h" )
# endif()

if( MSVC )
    SET( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup" )
endif()

# add_subdirectory(external/glm)
# add_subdirectory(external/yaml-cpp)
# add_subdirectory(external/Box2D-cmake)

# include_directories(external/fmt/include)

# include(FetchContent)
# add_subdirectory(external/fmt)
# FetchContent_Declare(fmt
#   GIT_REPOSITORY https://github.com/fmtlib/fmt.git
#   GIT_TAG master
# )
# FetchContent_MakeAvailable(fmt)

# add_subdirectory(external/spdlog)

# add_subdirectory(external/imgui)

# add_subdirectory(external/ImGuizmo)

add_subdirectory(src)


# if (ENGINE_BUILD_SANDBOX)
add_subdirectory(RayTracing)
add_subdirectory(Sandbox)

# default startup project for Visual Studio
if (MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Sandbox)
    set_property(TARGET Sandbox PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/Sandbox")
endif()

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(box2d REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(glm REQUIRED)
find_package(imguidocking REQUIRED)

target_link_libraries(${PROJECT_NAME}
    ${OPENGL_LIBRARIES}
    glfw
    Vulkan::Vulkan
    fmt::fmt
    spdlog::spdlog
    glm::glm
    box2d::box2d
    yaml-cpp::yaml-cpp
    imguidocking::imguidocking
    # ImGuizmo
)


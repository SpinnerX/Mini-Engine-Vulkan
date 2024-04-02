set(PROJECT_NAME Sandbox)

project(${PROJECT_NAME})
set(CMAKE_CXX_STANDARD 20)

include_directories(
    ../3DEngine
    ../3DEngine/Core
)

# set(CMAKE_PREFIX_PATH "C:\\Program Files (x86)\\3DEngine\\lib\\cmake")

# set(imgui_DIR "C:\\Program Files (x86)\\3DEngine\\lib\\cmake\\imgui")
# set(fmt_DIR "C:\\Program Files (x86)\\3DEngine\\libcmake\\fmt")
# set(spdlog_DIR "C:\\Program Files (x86)\\3DEngine\\libcmake\\spdlog")
# find_package(spdlog REQUIRED)
# find_package(fmt REQUIRED)
# find_package(imgui REQUIRED)
# find_package(ImGuizmo REQUIRED)
# find_package(fmt REQUIRED)
# find_package(spdlog REQUIRED)
# find_package(imgui REQUIRED)
# find_package(ImGuizmo REQUIRED)
# find_package(3DEngine REQUIRED)

add_executable(${PROJECT_NAME} Application.cpp)

target_link_libraries(
    ${PROJECT_NAME} PUBLIC
    Mini-GameEngine
    glfw
    # ${Vulkan_LIBRARIES}
    fmt
    spdlog
    glm
    tobanteGaming::Box2D
    yaml-cpp
    imgui
    ImGuizmo
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR})
set(
  all_src
  Mini-GameEngine/Application.cpp
)

set(
  all_headers
  ../include/Mini-GameEngine/Application.h
)

# Finding opengl, glfw, glew, glut, and possibly freetype and even freeglut
find_package(glfw3 3.4 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLUT REQUIRED)
find_package(FREEGLUT REQUIRED)

find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(glad REQUIRED)
find_package(glm REQUIRED)
find_package(Vulkan REQUIRED)

add_library(${PROJECT_NAME} ${all_src} ${all_headers})

# if(APPLE)

    # This is making sure we find the SDL libraries on mac
    # include_directories(${SDL2_INCLUDE_DIRS})

	if(VULKAN_FOUND)
		message("Vulkan found!")
		target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan)
	endif()
    # target_link_libraries(${PROJECT_NAME} pthread)
    # Linking the libraries for Cocoa, OpenGL, IOKit, and glfw
    target_link_libraries(${PROJECT_NAME}
      "-framework Cocoa"
      "-framework OpenGL"
      "-framework IOKit"
      glfw
      fmt::fmt
      glm::glm
      spdlog::spdlog
      ${OPENGL_gl_LIBRARY}
      GLEW::GLEW
      ${GLUT_LIBRARIES} # NOTE: If you want to use freeglut, then you need this to link freeglut to get working
      glad::glad
      ${APPKIT_LIBRARY} ${UNIFORMTYPEIDENTIFIERS_LIBRARY} # Note as TODO: Probably should put this somewhere else, since it relies on nfd.cmake...
    )
# endif(APPLE)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)


set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

if(MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES
    COMPILE_PDB_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
	COMPILE_PDB_NAME ${PROJECT_NAME}
  )

  install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/GameEngine.pdb"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    OPTIONAL
  )
endif()

if (BUILD_SHARED_LIBS)
	target_compile_definitions(${PROJECT_NAME}
    PUBLIC
	${PROJECT_NAME}_SHARED
  )
endif()

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "src" FILES ${all_src})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/../include" PREFIX "include" FILES ${all_headers})

install(
	TARGETS ${PROJECT_NAME}
	EXPORT ${PROJECT_NAME}Config
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
	EXPORT ${PROJECT_NAME}Config
	NAMESPACE ${PROJECT_NAME}::
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
  COMPATIBILITY SameMajorVersion
)

install(
	FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)

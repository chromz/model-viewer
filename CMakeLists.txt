cmake_minimum_required (VERSION 3.14)
project (ModelViewer)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
configure_file ("rex.obj" "rex.obj" COPYONLY)
configure_file ("rex.bmp" "rex.bmp" COPYONLY)
find_package (OpenGL REQUIRED)
find_package (SDL2 REQUIRED)
find_package (ASSIMP REQUIRED)
find_package (glm REQUIRED)
find_package (GLUT REQUIRED)
find_package (GLEW REQUIRED)

# Find Cglm
find_library (CGLM_LIB
  NAMES libcglm cglm
  PATHS /usr/lib)

find_path (CGLM_PATH
  NAMES cglm/cglm.h
  PATHS /usr/include)

message ("${CGLM_LIB} ${CGLM_PATH}")

if (NOT CGLM_LIB OR NOT CGLM_PATH)
	message (FATAL_ERROR "Socket io library or headers not found")
endif()


add_subdirectory (viewer)
add_subdirectory (cmd)


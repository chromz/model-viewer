cmake_minimum_required (VERSION 3.14)
project (ModelViewer)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
configure_file ("rex.obj" "rex.obj" COPYONLY)
configure_file ("rex.mtl" "rex.mtl" COPYONLY)
configure_file ("SpiderTex.jpg" "SpiderTex.jpg" COPYONLY)
configure_file ("drkwood2.jpg" "drkwood2.jpg" COPYONLY)
configure_file ("engineflare1.jpg" "engineflare1.jpg" COPYONLY)
configure_file ("wal67ar_small.jpg" "wal67ar_small.jpg" COPYONLY)
configure_file ("wal69ar_small.jpg" "wal69ar_small.jpg" COPYONLY)
# configure_file ("rex.bmp" "rex.bmp" COPYONLY)
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

find_library(SDL2_IMG_LIB
  NAMES libSDL2_image SDL2_image
  PATHS /usr/lib)

find_path(SDL2_IMG_PATH
  NAMES SDL2/SDL_image.h
  PATHS /usr/include)


if (NOT CGLM_LIB OR NOT CGLM_PATH)
	message (FATAL_ERROR "CGLM not found")
endif()

if (NOT SDL2_IMG_LIB OR NOT SDL2_IMG_PATH)
	message (FATAL_ERROR "SDL2_IMG not found")
endif()

add_subdirectory (viewer)
add_subdirectory (cmd)


set (VIEWER_SOURCES viewer.c)
find_package (OpenGL REQUIRED)
find_package (SDL2 REQUIRED)
find_package (ASSIMP REQUIRED)
add_library (viewer STATIC ${VIEWER_SOURCES})
target_include_directories (viewer PUBLIC
  ${SDL2_INCLUDE_DIRS}
  ${OPENGL_INCLUDE_DIRS}
  ${ASSIMP_INCLUDE_DIRS})
target_link_libraries (viewer PUBLIC
  ${SDL2_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${ASSIMP_LIBRARIES})

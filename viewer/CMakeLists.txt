set (VIEWER_SOURCES viewer.c)
add_library (viewer STATIC ${VIEWER_SOURCES})
find_package (GLUT REQUIRED)
target_include_directories (viewer PUBLIC
  SDL2::SDL2
  ${GLM_INCLUDE_DIRS}
  ${GLUT_INCLUDE_DIRS}
  ${OPENGL_INCLUDE_DIRS}
  ${ASSIMP_INCLUDE_DIRS})
target_link_libraries (viewer PUBLIC
  SDL2::SDL2
  ${GLUT_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${ASSIMP_LIBRARIES})

set (VIEWER_SOURCES viewer.c)
add_library (viewer STATIC ${VIEWER_SOURCES})
configure_file ("shader.vert" "../shader.vert" COPYONLY)
configure_file ("shader.frag" "../shader.frag" COPYONLY)
target_include_directories (viewer PUBLIC
  SDL2::SDL2
  ${GLM_INCLUDE_DIRS}
  ${GLEW_INCLUDE_DIRS}
  ${GLUT_INCLUDE_DIRS}
  ${OPENGL_INCLUDE_DIRS}
  ${ASSIMP_INCLUDE_DIRS}
  ${SDL2_IMG_PATH}
  ${CGLM_PATH})
target_link_libraries (viewer PUBLIC
  SDL2::SDL2
  ${GLUT_LIBRARIES}
  ${GLEW_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${ASSIMP_LIBRARIES}
  ${CGLM_LIB}
  ${SDL2_IMG_LIB})

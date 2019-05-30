
set (CMD_SOURCES main.c)
add_executable (model_viewer ${CMD_SOURCES})
target_include_directories(model_viewer PUBLIC
  ${PROJECT_SOURCE_DIR})
target_link_libraries(model_viewer viewer)


project(BufferPoolVisualizer VERSION 0.1.0)

add_executable(bp_viz buffer_pool_visualizer.cc)

if(TARGET SDL2::SDL2)
  # SDL2 built with cmake
  target_link_libraries(bp_viz hermes SDL2::SDL2)
else()
  # SDL2 built with autotools
  target_include_directories(bp_viz PRIVATE ${SDL2_INCLUDE_DIRS})
  target_link_libraries(bp_viz hermes ${SDL2_LIBRARIES})
endif()

add_custom_target(viz
  COMMAND LSAN_OPTIONS=suppressions=${CMAKE_CURRENT_SOURCE_DIR}/asan.supp
  ${CMAKE_BINARY_DIR}/bin/bp_viz
)

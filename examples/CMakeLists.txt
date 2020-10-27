
add_executable(end_to_end end_to_end.cc)
target_link_libraries(end_to_end hermes MPI::MPI_CXX
  $<$<BOOL:${HERMES_RPC_THALLIUM}>:thallium>)
target_compile_definitions(end_to_end
  PRIVATE $<$<BOOL:${HERMES_RPC_THALLIUM}>:HERMES_RPC_THALLIUM>)

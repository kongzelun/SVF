if(DEFINED IN_SOURCE_BUILD)
  add_llvm_tool(ddfg ddfg.cpp)
else()
  add_executable(ddfg ddfg.cpp)

  target_link_libraries(ddfg SvfLLVM ${llvm_libs})

  set_target_properties(ddfg PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                          ${CMAKE_BINARY_DIR}/bin)
endif()

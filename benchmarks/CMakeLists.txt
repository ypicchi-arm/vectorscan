include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR})

if (NOT FAT_RUNTIME AND (BUILD_SHARED_LIBS OR BUILD_STATIC_LIBS))
  add_executable(benchmarks benchmarks.cpp)
  set_source_files_properties(benchmarks.cpp PROPERTIES COMPILE_FLAGS
      "-Wall -Wno-unused-variable")
  target_link_libraries(benchmarks hs)

  add_executable(test_api test_extended_api.cpp)
  set_source_files_properties(test_extended_api.cpp PROPERTIES COMPILE_FLAGS
      "-Wall -Wno-unused-variable")
  target_link_libraries(test_api hs)
endif()

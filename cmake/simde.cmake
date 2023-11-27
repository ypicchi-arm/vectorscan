# include_directories(${PROJECT_SOURCE_DIR}/simde/simde)

pkg_check_modules(SIMDE simde)

if (SIMDE_FOUND)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DVS_SIMDE_BACKEND")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVS_SIMDE_BACKEND")

  if (SIMDE_NATIVE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DVS_SIMDE_NATIVE -DSIMDE_ENABLE_OPENMP -fopenmp-simd")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVS_SIMDE_NATIVE -DSIMDE_ENABLE_OPENMP -fopenmp-simd")
  endif()
else()
  message(FATAL_ERROR "SIMDe backend requested but SIMDe is not available on the system")
endif()

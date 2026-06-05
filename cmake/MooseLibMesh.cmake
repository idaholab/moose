# Discovery of a (possibly conda-provided) libMesh installation via libmesh-config.
#
# Mirrors framework/build.mk: libmesh-config is the single source of truth for the compiler
# and all flags (PETSc/MPI flags come in transitively through --libs/--ldflags). We honor
# LIBMESH_DIR from the environment and locate libmesh-config in the same priority order as
# build.mk, with the conda case (LIBMESH_DIR == CONDA_PREFIX) as the primary path.

# _moose_find_libmesh_config(<out_var>): installed/conda -> uninstalled in-tree -> PATH.
function(_moose_find_libmesh_config out_var)
  set(_candidates "")
  if(DEFINED ENV{LIBMESH_DIR})
    list(APPEND _candidates
      "$ENV{LIBMESH_DIR}/bin/libmesh-config"           # installed / conda layout
      "$ENV{LIBMESH_DIR}/contrib/bin/libmesh-config")  # uninstalled in-tree layout
  endif()
  foreach(_c IN LISTS _candidates)
    if(EXISTS "${_c}")
      set(${out_var} "${_c}" PARENT_SCOPE)
      return()
    endif()
  endforeach()
  find_program(_moose_lmc libmesh-config)
  if(_moose_lmc)
    set(${out_var} "${_moose_lmc}" PARENT_SCOPE)
    return()
  endif()
  message(FATAL_ERROR
    "Could not find libmesh-config. Set LIBMESH_DIR or activate the moose-dev conda env.")
endfunction()

# _moose_libmesh_query(<out_var> <args...>): run libmesh-config with METHOD set.
function(_moose_libmesh_query out_var)
  _moose_find_libmesh_config(_lmc)
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E env METHOD=${MOOSE_METHOD} ${_lmc} ${ARGN}
    OUTPUT_VARIABLE _out
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE _rc)
  if(NOT _rc EQUAL 0)
    message(FATAL_ERROR "libmesh-config ${ARGN} failed (METHOD=${MOOSE_METHOD})")
  endif()
  set(${out_var} "${_out}" PARENT_SCOPE)
endfunction()

# moose_libmesh_set_compilers(): set the toolchain from libmesh-config.
# MUST be called BEFORE project(), since libmesh-config --cxx returns mpicxx.
function(moose_libmesh_set_compilers)
  _moose_libmesh_query(_cxx --cxx)
  _moose_libmesh_query(_cc  --cc)
  _moose_libmesh_query(_fc  --fc)
  set(CMAKE_CXX_COMPILER     "${_cxx}" PARENT_SCOPE)
  set(CMAKE_C_COMPILER       "${_cc}"  PARENT_SCOPE)
  set(CMAKE_Fortran_COMPILER "${_fc}"  PARENT_SCOPE)
endfunction()

# moose_libmesh_create_target(): build the MOOSE::libmesh usage-requirements target.
# Call AFTER project().
function(moose_libmesh_create_target)
  if(TARGET MOOSE::libmesh)
    return()
  endif()

  _moose_libmesh_query(_cppflags --cppflags)
  _moose_libmesh_query(_cxxflags --cxxflags)
  _moose_libmesh_query(_include  --include)
  _moose_libmesh_query(_libs     --libs)
  _moose_libmesh_query(_ldflags  --ldflags)
  _moose_libmesh_query(_host     --host)
  set(MOOSE_LIBMESH_HOST "${_host}" CACHE INTERNAL "libMesh host triplet")

  separate_arguments(_cppflags_l NATIVE_COMMAND "${_cppflags}")
  separate_arguments(_cxxflags_l NATIVE_COMMAND "${_cxxflags}")
  separate_arguments(_include_l  NATIVE_COMMAND "${_include}")
  separate_arguments(_libs_l     NATIVE_COMMAND "${_libs}")
  separate_arguments(_ldflags_l  NATIVE_COMMAND "${_ldflags}")

  # Mirror build.mk: dedup link flags. libmesh-config embeds rpaths from each bundled
  # dependency (libmesh, timpi, petsc) sharing a prefix, producing duplicate -rpath
  # entries that macOS ld warns about. REMOVE_DUPLICATES keeps first occurrence + order.
  list(REMOVE_DUPLICATES _libs_l)
  list(REMOVE_DUPLICATES _ldflags_l)

  add_library(MOOSE::libmesh INTERFACE IMPORTED GLOBAL)
  set_target_properties(MOOSE::libmesh PROPERTIES
    # includes + cpp defines apply to all languages; cxxflags only to C++.
    INTERFACE_COMPILE_OPTIONS
      "${_include_l};${_cppflags_l};$<$<COMPILE_LANGUAGE:CXX>:${_cxxflags_l}>"
    INTERFACE_LINK_OPTIONS    "${_ldflags_l}"
    INTERFACE_LINK_LIBRARIES  "${_libs_l}")
endfunction()

# Python extension modules built by MOOSE: the HIT parser binding (hit.so) and the
# capabilities binding (_pycapabilities.so). These mirror framework/moose.mk verbatim via
# custom commands rather than add_library, because:
#   * they are plain-named .so modules (.so is in Python's EXTENSION_SUFFIXES, so importable),
#   * _pycapabilities must be built with the UNDERLYING (non-MPI) compiler and special
#     defines so it does not drag in MPI/libMesh -- which does not fit CMake's
#     one-compiler-per-project model.
# hit.cpp is pre-generated and committed, so no Cython step is required.
#
# Outputs land in ${CMAKE_BINARY_DIR}/python-ext so the source tree (and the coexisting make
# build's own in-source artifacts) are left untouched.

function(moose_add_python_extensions)
  set(_out "${CMAKE_BINARY_DIR}/python-ext")
  file(MAKE_DIRECTORY "${_out}")

  set(_hit "${CMAKE_SOURCE_DIR}/framework/contrib/hit")
  set(_pycap "${CMAKE_SOURCE_DIR}/python/pycapabilities")
  set(_fwk "${CMAKE_SOURCE_DIR}/framework")
  set(_wasp_dir "$ENV{WASP_DIR}")

  # Python compile/link flags (python3-config --includes / --prefix).
  execute_process(COMMAND python3-config --includes
    OUTPUT_VARIABLE _py_inc OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(COMMAND python3-config --prefix
    OUTPUT_VARIABLE _py_prefix OUTPUT_STRIP_TRAILING_WHITESPACE)
  separate_arguments(_py_inc_l NATIVE_COMMAND "${_py_inc}")
  set(_pymod_flags ${_py_inc_l} -L${_py_prefix}/lib -Wl,-rpath,${_py_prefix}/lib)

  if(APPLE)
    set(_dynlookup -undefined dynamic_lookup)
  else()
    set(_dynlookup "")
  endif()

  # The underlying (non-MPI) compiler, for _pycapabilities (mpicxx -show | first word).
  execute_process(COMMAND ${CMAKE_CXX_COMPILER} -show
    OUTPUT_VARIABLE _show OUTPUT_STRIP_TRAILING_WHITESPACE RESULT_VARIABLE _rc)
  if(_rc EQUAL 0)
    separate_arguments(_show_l NATIVE_COMMAND "${_show}")
    list(GET _show_l 0 _underlying_cxx)
  else()
    set(_underlying_cxx "${CMAKE_CXX_COMPILER}")
  endif()

  # ---- hit.so (pyhit) ----
  set(_hit_so "${_out}/hit.so")
  add_custom_command(
    OUTPUT "${_hit_so}"
    COMMAND ${CMAKE_CXX_COMPILER} -I${_hit}/include -std=c++17 -w -fPIC -shared
            ${_hit}/src/hit/hit.cpp ${_hit}/src/hit/lex.cc
            ${_hit}/src/hit/parse.cc ${_hit}/src/hit/braceexpr.cc
            ${_pymod_flags} -I${_wasp_dir}/include
            -Wl,-rpath,${_wasp_dir}/lib -L${_wasp_dir}/lib -lwasphit -lwaspcore
            ${_dynlookup} -o ${_hit_so}
    DEPENDS "${_hit}/src/hit/hit.cpp" "${_hit}/src/hit/parse.cc"
            "${_hit}/src/hit/lex.cc" "${_hit}/src/hit/braceexpr.cc"
    COMMENT "Building python extension hit.so"
    VERBATIM)

  # ---- _pycapabilities.so ----
  set(_pycap_so "${_out}/_pycapabilities.so")
  add_custom_command(
    OUTPUT "${_pycap_so}"
    COMMAND ${_underlying_cxx} -DFOR_PYCAPABILITIES -DMOOSESTRINGUTILS_NO_LIBMESH
            -std=c++17 -w -fPIC -shared
            ${_pycap}/_pycapabilities.C
            ${_fwk}/src/base/Capability.C
            ${_fwk}/src/base/CapabilityException.C
            ${_fwk}/src/base/CapabilityRegistry.C
            ${_pymod_flags}
            -I${_fwk}/contrib/cpp-peglib/include
            -I${_fwk}/include/base -I${_fwk}/include/utils
            ${_dynlookup} -o ${_pycap_so}
    DEPENDS "${_pycap}/_pycapabilities.C" "${_fwk}/src/base/Capability.C"
            "${_fwk}/src/base/CapabilityException.C" "${_fwk}/src/base/CapabilityRegistry.C"
    COMMENT "Building python extension _pycapabilities.so"
    VERBATIM)

  add_custom_target(moose-python-extensions ALL
    DEPENDS "${_hit_so}" "${_pycap_so}")
endfunction()

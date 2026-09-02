# METHOD handling for the MOOSE CMake build.
#
# MOOSE co-installs one library per METHOD (opt/dbg/devel/oprof/prof), each linking a
# different libMesh flavor. We therefore use one CMake build tree per METHOD (selected via
# -DMOOSE_METHOD=...), NOT CMAKE_BUILD_TYPE. METHOD is baked into the cache, every
# libmesh-config query, and every output name.

set(MOOSE_VALID_METHODS opt dbg devel oprof prof)

if(NOT DEFINED MOOSE_METHOD)
  if(DEFINED ENV{METHOD})
    set(_moose_default_method "$ENV{METHOD}")
  else()
    set(_moose_default_method "opt")
  endif()
  set(MOOSE_METHOD "${_moose_default_method}" CACHE STRING
      "MOOSE build method / libMesh flavor (one of: ${MOOSE_VALID_METHODS})")
endif()
set_property(CACHE MOOSE_METHOD PROPERTY STRINGS ${MOOSE_VALID_METHODS})

if(NOT MOOSE_METHOD IN_LIST MOOSE_VALID_METHODS)
  message(FATAL_ERROR
    "MOOSE_METHOD='${MOOSE_METHOD}' is invalid; choose one of: ${MOOSE_VALID_METHODS}")
endif()

# moose_method_libname(<out_var> <base>) -> "<base>-<method>" (e.g. libmoose -> moose-opt)
function(moose_method_libname out_var base)
  set(${out_var} "${base}-${MOOSE_METHOD}" PARENT_SCOPE)
endfunction()

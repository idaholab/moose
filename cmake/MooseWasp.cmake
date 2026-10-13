# Discover a (conda-provided) WASP install and expose it as MOOSE::wasp.
#
# Mirrors framework/moose.mk: WASP_DIR defaults from the environment (the moose-dev conda
# env sets it to $CONDA_PREFIX); we link every libwasp<name> shared lib and add -DWASP_ENABLED
# plus the include dir and an rpath.

function(moose_create_wasp_target)
  if(TARGET MOOSE::wasp)
    return()
  endif()
  if(NOT DEFINED ENV{WASP_DIR})
    message(FATAL_ERROR
      "WASP_DIR not set; activate the moose-dev conda env or set WASP_DIR to a WASP install.")
  endif()
  set(_wasp_dir "$ENV{WASP_DIR}")
  set(_lib "${_wasp_dir}/lib")
  set(_inc "${_wasp_dir}/include")
  if(NOT EXISTS "${_inc}/waspcore")
    message(FATAL_ERROR "WASP includes not found under ${_inc}")
  endif()

  # Keep only the unversioned libwasp<name> symlinks (e.g. libwaspcore.dylib), not the
  # version-suffixed copies (libwaspcore.4.4.0.dylib).
  file(GLOB _candidates "${_lib}/libwasp*${CMAKE_SHARED_LIBRARY_SUFFIX}")
  set(_libs "")
  foreach(_l IN LISTS _candidates)
    get_filename_component(_n "${_l}" NAME)
    if(_n MATCHES "^libwasp[a-z]+\\${CMAKE_SHARED_LIBRARY_SUFFIX}$")
      list(APPEND _libs "${_l}")
    endif()
  endforeach()
  if(NOT _libs)
    message(FATAL_ERROR "No WASP libraries found in ${_lib}")
  endif()

  add_library(MOOSE::wasp INTERFACE IMPORTED GLOBAL)
  set_target_properties(MOOSE::wasp PROPERTIES
    INTERFACE_COMPILE_DEFINITIONS "WASP_ENABLED"
    INTERFACE_INCLUDE_DIRECTORIES "${_inc}"
    INTERFACE_LINK_LIBRARIES      "${_libs}"
    INTERFACE_LINK_OPTIONS        "-Wl,-rpath,${_lib}")
endfunction()

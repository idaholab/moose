# moose_add_app(): the CMake analogue of `include $(FRAMEWORK_DIR)/app.mk`.
#
#   moose_add_app(NAME <name> APP_DIR <dir> [BUILD_EXEC] [DEPEND_MODULES m1 m2 ...])
#
# Builds lib<name>-<method> (SHARED) from <dir>/src (excluding main.C) with a native GROUP
# unity build (one group per top-level src subdir, as the make build does), links Moose::moose
# plus any dependent module libraries, and exposes a Moose::<name> alias. With BUILD_EXEC it
# also builds the executable <name>-<method> from <dir>/src/main.C.
#
# The created target names are returned to the caller in:
#   MOOSE_APP_LIB_TARGET   (always)
#   MOOSE_APP_EXEC_TARGET  (only when BUILD_EXEC was given)
#
# This is the downstream front door for the new build; legacy apps that `include app.mk`
# keep working unchanged during the migration window.
function(moose_add_app)
  cmake_parse_arguments(A "BUILD_EXEC" "NAME;APP_DIR" "DEPEND_MODULES" ${ARGN})
  if(NOT A_NAME OR NOT A_APP_DIR)
    message(FATAL_ERROR "moose_add_app: NAME and APP_DIR are required")
  endif()

  set(_srcdir "${A_APP_DIR}/src")
  file(GLOB_RECURSE _src CONFIGURE_DEPENDS "${_srcdir}/*.C")
  set(_main "${_srcdir}/main.C")
  list(REMOVE_ITEM _src "${_main}")

  # --- application library ---
  moose_method_libname(_lib ${A_NAME})
  add_library(${_lib} SHARED ${_src})
  add_library(Moose::${A_NAME} ALIAS ${_lib})

  moose_collect_subdirs(_inc "${A_APP_DIR}/include")
  target_include_directories(${_lib} PUBLIC ${_inc})

  set(_deps Moose::moose)
  foreach(_m IN LISTS A_DEPEND_MODULES)
    list(APPEND _deps Moose::${_m})
  endforeach()
  target_link_libraries(${_lib} PUBLIC ${_deps})

  # unity: GROUP per top-level src subdir
  set_target_properties(${_lib} PROPERTIES UNITY_BUILD ON UNITY_BUILD_MODE GROUP)
  foreach(_f IN LISTS _src)
    if(_f MATCHES "${_srcdir}/([^/]+)/")
      set_source_files_properties("${_f}" PROPERTIES UNITY_GROUP "src_${CMAKE_MATCH_1}")
    else()
      set_source_files_properties("${_f}" PROPERTIES SKIP_UNITY_BUILD_INCLUSION TRUE)
    endif()
  endforeach()

  set(MOOSE_APP_LIB_TARGET "${_lib}" PARENT_SCOPE)

  # --- executable ---
  if(A_BUILD_EXEC)
    set(_exec "${_lib}-exec")        # distinct CMake target name; file name is <name>-<method>
    add_executable(${_exec} "${_main}")
    set_target_properties(${_exec} PROPERTIES OUTPUT_NAME "${_lib}")
    target_link_libraries(${_exec} PRIVATE Moose::${A_NAME})
    set(MOOSE_APP_EXEC_TARGET "${_exec}" PARENT_SCOPE)
  endif()
endfunction()

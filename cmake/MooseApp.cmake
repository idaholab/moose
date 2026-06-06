# moose_add_app(): the CMake analogue of `include $(FRAMEWORK_DIR)/app.mk`.
#
#   moose_add_app(NAME <name> APP_DIR <dir>
#                 [BUILD_EXEC] [WITH_TEST_LIB] [DEPEND_MODULES m1 m2 ...])
#
# Builds lib<name>-<method> (SHARED) from <dir>/src (excluding main.C) with a native GROUP
# unity build (one group per top-level src subdir, as the make build does), defines
# <NAME>_ENABLED (PUBLIC, so consumers/dependers see it, mirroring app.mk's cumulative
# -D<NAME>_ENABLED), links Moose::moose plus any dependent module libraries, and exposes a
# Moose::<name> alias.
#
# WITH_TEST_LIB additionally compiles <dir>/test/src into lib<name>_test-<method> (the test
# objects + the <Name>TestApp) -- present for modules, whose main.C instantiates <Name>TestApp.
# BUILD_EXEC builds the executable <name>-<method> from <dir>/src/main.C, linked against the
# test lib when one exists (else the app lib). Both are no-ops when the relevant files are
# absent, so the same call works for any app.
#
# Created target names are returned in MOOSE_APP_LIB_TARGET / MOOSE_APP_EXEC_TARGET.
#
# This is the downstream front door for the new build; legacy apps that `include app.mk`
# keep working unchanged during the migration window.
function(moose_add_app)
  cmake_parse_arguments(A "BUILD_EXEC;WITH_TEST_LIB;GEN_REVISION" "NAME;APP_DIR" "DEPEND_MODULES" ${ARGN})
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

  # Per-app revision header <Camel>Revision.h (app.mk's GEN_REVISION=yes), generated at
  # configure time into a build-only include dir made PUBLIC (so test libs/dependers see it).
  if(A_GEN_REVISION)
    _moose_camel(_camel "${A_NAME}")
    set(_revdir "${CMAKE_CURRENT_BINARY_DIR}/${A_NAME}_generated_include")
    file(MAKE_DIRECTORY "${_revdir}")
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E env
        REPO_LOCATION=${A_APP_DIR}
        HEADER_FILE=${_revdir}/${_camel}Revision.h
        APPLICATION_NAME=${A_NAME}
        INSTALLABLE_DIRS=
        ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/framework/scripts/get_repo_revision.py
      RESULT_VARIABLE _rrc)
    if(NOT _rrc EQUAL 0)
      message(FATAL_ERROR "moose_add_app(${A_NAME}): failed to generate ${_camel}Revision.h")
    endif()
    target_include_directories(${_lib} PUBLIC "${_revdir}")
  endif()

  # -D<NAME>_ENABLED (uppercased, dashes removed), as app.mk defines per app.
  string(TOUPPER "${A_NAME}" _upper)
  string(REPLACE "-" "" _upper "${_upper}")
  target_compile_definitions(${_lib} PUBLIC ${_upper}_ENABLED)

  set(_deps Moose::moose)
  foreach(_m IN LISTS A_DEPEND_MODULES)
    list(APPEND _deps Moose::${_m})
  endforeach()
  target_link_libraries(${_lib} PUBLIC ${_deps})
  _moose_unity_by_subdir(${_lib} "${_srcdir}" "${_src}")

  set(MOOSE_APP_LIB_TARGET "${_lib}" PARENT_SCOPE)

  # --- test-object library (modules: <dir>/test/src -> lib<name>_test-<method>) ---
  set(_exec_link Moose::${A_NAME})
  if(A_WITH_TEST_LIB)
    set(_tsrcdir "${A_APP_DIR}/test/src")
    file(GLOB_RECURSE _tsrc CONFIGURE_DEPENDS "${_tsrcdir}/*.C")
    if(_tsrc)
      moose_method_libname(_tlib ${A_NAME}_test)
      add_library(${_tlib} SHARED ${_tsrc})
      add_library(Moose::${A_NAME}_test ALIAS ${_tlib})
      target_link_libraries(${_tlib} PUBLIC Moose::${A_NAME})
      moose_collect_subdirs(_tinc "${A_APP_DIR}/test/include")
      target_include_directories(${_tlib} PUBLIC ${_tinc})
      _moose_unity_by_subdir(${_tlib} "${_tsrcdir}" "${_tsrc}")
      set(_exec_link Moose::${A_NAME}_test)
    endif()
  endif()

  # --- executable ---
  if(A_BUILD_EXEC AND EXISTS "${_main}")
    set(_exec "${_lib}-exec")        # distinct CMake target name; file name is <name>-<method>
    add_executable(${_exec} "${_main}")
    set_target_properties(${_exec} PROPERTIES OUTPUT_NAME "${_lib}")
    target_link_libraries(${_exec} PRIVATE ${_exec_link})
    set(MOOSE_APP_EXEC_TARGET "${_exec}" PARENT_SCOPE)
  endif()
endfunction()

# _moose_camel(<out> <name>): snake_case -> CamelCase (combined -> Combined,
# heat_transfer -> HeatTransfer), matching app.mk's perl s/(?:^|_)([a-z])/\u$1/g.
function(_moose_camel out name)
  string(REPLACE "_" ";" _parts "${name}")
  set(_r "")
  foreach(_p IN LISTS _parts)
    string(SUBSTRING "${_p}" 0 1 _head)
    string(TOUPPER "${_head}" _head)
    string(SUBSTRING "${_p}" 1 -1 _tail)
    string(APPEND _r "${_head}${_tail}")
  endforeach()
  set(${out} "${_r}" PARENT_SCOPE)
endfunction()

# Apply a native GROUP unity build to <target>: one group per top-level subdir of <srcroot>;
# sources not under such a subdir are compiled standalone (not unity-safe in general).
function(_moose_unity_by_subdir target srcroot srcs)
  set_target_properties(${target} PROPERTIES UNITY_BUILD ON UNITY_BUILD_MODE GROUP)
  foreach(_f IN LISTS srcs)
    if(_f MATCHES "${srcroot}/([^/]+)/")
      set_source_files_properties("${_f}" PROPERTIES UNITY_GROUP "g_${CMAKE_MATCH_1}")
    else()
      set_source_files_properties("${_f}" PROPERTIES SKIP_UNITY_BUILD_INCLUSION TRUE)
    endif()
  endforeach()
endfunction()

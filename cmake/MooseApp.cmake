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

  # BUILD_INTERFACE-scoped so absolute source paths don't leak into the install export; the
  # installed MooseConfig re-adds module headers from include/moose/modules/<name>.
  moose_collect_subdirs(_inc "${A_APP_DIR}/include")
  set(_binc "")
  foreach(_d IN LISTS _inc)
    list(APPEND _binc "$<BUILD_INTERFACE:${_d}>")
  endforeach()
  target_include_directories(${_lib} PUBLIC ${_binc})

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
    target_include_directories(${_lib} PUBLIC "$<BUILD_INTERFACE:${_revdir}>")
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

  # --- plugins (app.mk PLUGIN_DIR -> <name>-<method>.plugin) ---
  # Each source in <dir>/plugins and <dir>/test/plugins becomes a standalone shared object
  # named <basename>-<method>.plugin, written into the source plugins dir (MOOSE loads it from
  # there by the path in the input file; .plugin is gitignored). Symbols are resolved against
  # the host executable at load (-undefined dynamic_lookup), so plugins are NOT linked to
  # moose/libmesh -- they only borrow the app's include dirs + compile options. Building the
  # app library also builds its plugins.
  _moose_add_plugins(${_lib} "${A_NAME}" "${A_APP_DIR}")

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

# _moose_add_plugins(<applib> <name> <appdir>): build every plugin source under
# <appdir>/plugins and <appdir>/test/plugins as <basename>-<method>.plugin (a MODULE in the
# source dir), and make <applib> depend on them so they build with the app.
function(_moose_add_plugins applib name appdir)
  set(_plugin_targets "")
  # test/include dirs (e.g. abaqus utility .f files INCLUDEd by UMAT plugins) -- these are on
  # the test lib, not the app lib, but plugins live in test/plugins and reference them.
  moose_collect_subdirs(_test_inc "${appdir}/test/include")
  foreach(_pdir IN ITEMS "${appdir}/plugins" "${appdir}/test/plugins")
    if(NOT IS_DIRECTORY "${_pdir}")
      continue()
    endif()
    file(GLOB_RECURSE _psrc CONFIGURE_DEPENDS
         "${_pdir}/*.C" "${_pdir}/*.c" "${_pdir}/*.f" "${_pdir}/*.f90")
    foreach(_ps IN LISTS _psrc)
      get_filename_component(_pn "${_ps}" NAME_WE)
      get_filename_component(_pd "${_ps}" DIRECTORY)
      string(REGEX REPLACE "[^A-Za-z0-9]" "_" _ptag "${name}_${_pd}_${_pn}")
      set(_ptgt "plugin_${_ptag}")
      add_library(${_ptgt} MODULE "${_ps}")
      set_target_properties(${_ptgt} PROPERTIES
        PREFIX ""
        SUFFIX "-${MOOSE_METHOD}.plugin"
        OUTPUT_NAME "${_pn}"
        LIBRARY_OUTPUT_DIRECTORY "${_pd}")
      # Borrow the app's include dirs + compile options (e.g. -include MooseConfig.h, the
      # libmesh CXX flags) without linking any libraries; CXX-only flags are genex-guarded so
      # Fortran/C plugins ignore them.
      target_include_directories(${_ptgt} PRIVATE
        "${_pd}" ${_test_inc}
        $<TARGET_PROPERTY:${applib},INCLUDE_DIRECTORIES>)
      target_compile_options(${_ptgt} PRIVATE
        $<TARGET_PROPERTY:${applib},INTERFACE_COMPILE_OPTIONS>
        # Fortran flags build.mk adds to libmesh_FFLAGS for gfortran. -fdefault-real-8/
        # -fdefault-double-8 are essential: Abaqus UMAT code uses default-kind REALs expecting
        # 8 bytes; without these the plugin computes garbage (zero-pivot / NaN at solve time).
        "$<$<COMPILE_LANGUAGE:Fortran>:-fdefault-real-8;-fdefault-double-8;-ffree-line-length-none>")
      if(APPLE)
        target_link_options(${_ptgt} PRIVATE -undefined dynamic_lookup)
      endif()
      list(APPEND _plugin_targets ${_ptgt})
    endforeach()
  endforeach()
  if(_plugin_targets)
    add_dependencies(${applib} ${_plugin_targets})
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

# Builds libmoose-<method> from framework/src (+ the in-tree contrib source dirs), with a
# native GROUP unity build mirroring the make build's per-top-level-subdir grouping.

set(_fwk "${CMAKE_CURRENT_SOURCE_DIR}")

# --------------------------------------------------------------------------------------
# Generated headers: MooseConfig.h + MooseRevision.h (into a build-only include dir).
# --------------------------------------------------------------------------------------
set(MOOSE_CONFIG_DIR "${CMAKE_CURRENT_BINARY_DIR}/moose_generated_include")
file(MAKE_DIRECTORY "${MOOSE_CONFIG_DIR}")

# No ./configure was run (conda build): the default config IS the config, exactly as the
# make build's fallback copies MooseDefaultConfig.h -> MooseConfig.h.
configure_file("${_fwk}/include/base/MooseDefaultConfig.h"
               "${MOOSE_CONFIG_DIR}/MooseConfig.h" COPYONLY)
set(MOOSE_CONFIG_HEADER "${MOOSE_CONFIG_DIR}/MooseConfig.h")
# Expose to the top-level scope (moose_install lives there).
set(MOOSE_CONFIG_DIR "${MOOSE_CONFIG_DIR}" CACHE INTERNAL "MOOSE generated header dir")
set(MOOSE_CONFIG_HEADER "${MOOSE_CONFIG_HEADER}" CACHE INTERNAL "Generated MooseConfig.h path")

# MooseRevision.h via the existing python generator. Generated at configure time so it
# exists before the build; the script only rewrites when the revision actually changes.
execute_process(
  COMMAND ${CMAKE_COMMAND} -E env
    REPO_LOCATION=${_fwk}
    HEADER_FILE=${MOOSE_CONFIG_DIR}/MooseRevision.h
    APPLICATION_NAME=MOOSE
    INSTALLABLE_DIRS=
    ${Python3_EXECUTABLE} ${_fwk}/scripts/get_repo_revision.py
  RESULT_VARIABLE _rev_rc)
if(NOT _rev_rc EQUAL 0)
  message(FATAL_ERROR "Failed to generate MooseRevision.h")
endif()

# --------------------------------------------------------------------------------------
# Contrib libraries (pcre linked into libmoose; hit + gtest standalone).
# --------------------------------------------------------------------------------------
moose_add_pcre()
moose_add_hit()
moose_add_gtest()

# --------------------------------------------------------------------------------------
# libmoose sources: framework/src minus only src/kokkos (Kokkos .K/.C needs the dedicated
# Kokkos toolchain; not done at this time), plus the in-tree contrib source dirs that
# compile into libmoose. NOTE: src/libtorch, src/mfem, src/neml2 ARE included even when those
# features are disabled -- their internals are #ifdef-guarded but they carry unconditional
# registrations (e.g. NEML2ActionCommon), exactly as the make build compiles them.
# --------------------------------------------------------------------------------------
file(GLOB_RECURSE _moose_src CONFIGURE_DEPENDS "${_fwk}/src/*.C")
list(FILTER _moose_src EXCLUDE REGEX "/src/kokkos/")

foreach(_cdir mtwist pugixml minijson/src/minijson tinyhttp/src/tinyhttp)
  file(GLOB_RECURSE _cs CONFIGURE_DEPENDS
       "${_fwk}/contrib/${_cdir}/*.C"
       "${_fwk}/contrib/${_cdir}/*.cc"
       "${_fwk}/contrib/${_cdir}/*.c")
  list(APPEND _moose_src ${_cs})
endforeach()

moose_method_libname(_moose moose)
add_library(${_moose} SHARED ${_moose_src})
add_library(Moose::moose ALIAS ${_moose})

# --------------------------------------------------------------------------------------
# Include directories: every subdir of framework/include + each contrib/*/include
# (excluding the optional-feature contribs), gtest, hit, and the generated headers.
# --------------------------------------------------------------------------------------
moose_collect_subdirs(_inc_dirs "${_fwk}/include")
set(_ignore_contrib libtorch mfem neml2 kokkos)
file(GLOB _contribs LIST_DIRECTORIES true "${_fwk}/contrib/*")
foreach(_c IN LISTS _contribs)
  if(IS_DIRECTORY "${_c}/include")
    get_filename_component(_cname "${_c}" NAME)
    if(NOT _cname IN_LIST _ignore_contrib)
      moose_collect_subdirs(_csub "${_c}/include")
      list(APPEND _inc_dirs ${_csub})
    endif()
  endif()
endforeach()
list(APPEND _inc_dirs
  "${_fwk}/contrib/gtest"
  "${_fwk}/contrib/hit/include")

# Build-tree include dirs are absolute source paths -- scope them to BUILD_INTERFACE so they
# don't leak into the install export. The installed MooseConfig.cmake re-adds the equivalent
# dirs from $PREFIX/include/moose (globbed, same as the build does).
set(_binc "$<BUILD_INTERFACE:${MOOSE_CONFIG_DIR}>")  # generated MooseConfig.h / MooseRevision.h
foreach(_d IN LISTS _inc_dirs)
  list(APPEND _binc "$<BUILD_INTERFACE:${_d}>")
endforeach()
target_include_directories(${_moose} PUBLIC ${_binc})

# --------------------------------------------------------------------------------------
# Compile usage requirements not carried by libmesh-config.
# --------------------------------------------------------------------------------------
# -DMETHOD=<method> (build.mk adds this to libmesh_CXXFLAGS, not libmesh-config).
target_compile_definitions(${_moose} PUBLIC "METHOD=${MOOSE_METHOD}")

# Force-include the generated MooseConfig.h, as the make build does via -include. Scoped to
# BUILD_INTERFACE; the installed config adds the installed MooseConfig.h instead.
target_compile_options(${_moose} PUBLIC
  "$<BUILD_INTERFACE:$<$<COMPILE_LANGUAGE:CXX>:SHELL:-include ${MOOSE_CONFIG_HEADER}>>")

# FParser JIT include path string (PRIVATE: only libmoose sources reference it).
set(_adfparser "-I${_fwk}/include/utils -I${_fwk}/include/base ${MOOSE_LIBMESH_INCLUDE}")
target_compile_definitions(${_moose} PRIVATE "ADFPARSER_INCLUDES=\"${_adfparser}\"")

# --------------------------------------------------------------------------------------
# Link: libMesh + WASP + hit are public usage requirements; pcre is absorbed into libmoose.
# (The make build links hit only at the app stage; folding it in here is more correct for a
# shared library, which should declare its own dependencies.)
# -fopenmp: the make build passes libmesh_CXXFLAGS -- which include -fopenmp -- on its link
# line, pulling in the OpenMP runtime (MooseInit calls omp_set_num_threads).
# --------------------------------------------------------------------------------------
target_link_libraries(${_moose}
  PUBLIC  MOOSE::libmesh MOOSE::wasp Moose::hit
  PRIVATE Moose::pcre)
target_link_options(${_moose} PUBLIC -fopenmp)

# --------------------------------------------------------------------------------------
# Native unity build: GROUP mode, one group per top-level src subdir (per-contrib for the
# contrib sources), mirroring the make build.
# --------------------------------------------------------------------------------------
set_target_properties(${_moose} PROPERTIES
  UNITY_BUILD ON
  UNITY_BUILD_MODE GROUP)
# Only framework/src is unity-grouped (one group per top-level subdir). The in-tree contrib
# sources are compiled standalone, exactly as the make build does (some, e.g. mtwist, are
# not unity-safe -- two TUs define the same static symbol).
foreach(_f IN LISTS _moose_src)
  if(_f MATCHES "${_fwk}/src/([^/]+)/")
    set_source_files_properties("${_f}" PROPERTIES UNITY_GROUP "src_${CMAKE_MATCH_1}")
  else()
    set_source_files_properties("${_f}" PROPERTIES SKIP_UNITY_BUILD_INCLUSION TRUE)
  endif()
endforeach()

message(STATUS "libmoose target: ${_moose} (${CMAKE_CURRENT_BINARY_DIR})")

# --------------------------------------------------------------------------------------
# exodiff: the Exodus diffing tool (its own flags; links only libMesh, not libmoose).
# --------------------------------------------------------------------------------------
file(GLOB_RECURSE _exodiff_src CONFIGURE_DEPENDS "${_fwk}/contrib/exodiff/*.C")
add_executable(exodiff ${_exodiff_src})
target_include_directories(exodiff PRIVATE "${_fwk}/contrib/exodiff")
target_compile_options(exodiff PRIVATE -std=c++14 -O2 -felide-constructors -w)
target_link_libraries(exodiff PRIVATE MOOSE::libmesh)
# Emit exodiff where the TestHarness looks for an in-tree build
# (python/TestHarness/testers/Exodiff.py -> framework/contrib/exodiff/exodiff). Method-
# independent and gitignored. Building any METHOD's tree refreshes this single binary.
set_target_properties(exodiff PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${_fwk}/contrib/exodiff")

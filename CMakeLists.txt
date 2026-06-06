# Phase 0 scaffolding for the MOOSE GNU Make -> CMake transition.
#
# This CMake build COEXISTS with the existing framework/*.mk build and does not replace it.
# It is driven one build tree per METHOD: cmake -S . -B build/<method> -DMOOSE_METHOD=<method>.
# libMesh / PETSc / WASP are discovered from the environment (LIBMESH_DIR/PETSC_DIR/WASP_DIR),
# which the moose-dev conda env provides. See the migration roadmap for the full plan.

cmake_minimum_required(VERSION 3.24)

list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# METHOD must be resolved before we query libmesh-config for the compiler.
include(MooseMethods)
include(MooseUtils)

# libMesh is the source of truth for the toolchain; the compiler (mpicxx/mpicc/mpif90)
# must be set before project().
include(MooseLibMesh)
moose_libmesh_set_compilers()

project(Moose VERSION 1.0 LANGUAGES CXX C Fortran)

message(STATUS "MOOSE build method:    ${MOOSE_METHOD}")
message(STATUS "libMesh CXX compiler:  ${CMAKE_CXX_COMPILER}")

# Build the MOOSE::libmesh usage-requirements target from libmesh-config.
moose_libmesh_create_target()
message(STATUS "libMesh host triplet:  ${MOOSE_LIBMESH_HOST}")

# WASP (conda-provided) and Python (for generated headers / bindings).
include(MooseWasp)
moose_create_wasp_target()
find_package(Python3 REQUIRED COMPONENTS Interpreter)

# Contrib library helpers (pcre/hit/gtest) and the downstream app helper.
include(MooseContrib)
include(MooseApp)

# Phase 0 verification: a tiny TU that includes and links libMesh. Off by default; enabled
# with -DMOOSE_SANITY_CHECK=ON.
option(MOOSE_SANITY_CHECK "Build the Phase 0 libMesh sanity executable" OFF)
if(MOOSE_SANITY_CHECK)
  moose_method_libname(_sanity_name moose_libmesh_sanity)
  add_executable(${_sanity_name} cmake/checks/libmesh_sanity.C)
  target_link_libraries(${_sanity_name} PRIVATE MOOSE::libmesh)
endif()

# Phase 1: the MOOSE framework library and unit-test executable.
add_subdirectory(framework)
add_subdirectory(unit)

# Phase 2: the MOOSE test application, built via moose_add_app().
add_subdirectory(test)

# Phase 3: all physics modules (libraries), built via moose_add_app() with the dep graph.
option(MOOSE_ENABLE_MODULES "Build and install the physics modules" ON)
if(MOOSE_ENABLE_MODULES)
  add_subdirectory(modules)
else()
  set(MOOSE_ENABLED_MODULES "" CACHE INTERNAL "Enabled MOOSE module names" FORCE)
endif()

# Phase 4: export a build-tree CMake package + define a relocatable install, both consumable
# via find_package(Moose).
include(MooseExport)
moose_export()
include(MooseInstall)
moose_install()

# Python extension modules (hit.so, _pycapabilities.so).
include(MoosePython)
moose_add_python_extensions()

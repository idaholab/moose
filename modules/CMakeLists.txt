# All MOOSE physics modules, built as libmodule-<method> via moose_add_app().
# Mirrors modules/modules.mk: the module list, the inter-module
# dependency graph, and the auto-enable behaviour (enabling a module enables its deps).
#
# Each module is an option (default ON == ALL_MODULES). Targets are created in dependency
# order so each module's Moose::<dep> alias exists before a depender links it.
#
# NOTE: this file builds the module LIBRARIES. The `combined` executable (which links all
# modules + CombinedTestApp and registers everything) needs test-object-library support and
# is handled differently.

# Dependency graph (module -> direct module deps). Modules not listed have no module deps.
set(_dep_fluid_properties   misc)
set(_dep_heat_transfer      ray_tracing)
set(_dep_solid_properties   heat_transfer)
set(_dep_navier_stokes      fluid_properties rdg heat_transfer)
set(_dep_contact            solid_mechanics)
set(_dep_fsi                navier_stokes solid_mechanics)
set(_dep_peridynamics       solid_mechanics)
set(_dep_phase_field        solid_mechanics)
set(_dep_porous_flow        solid_mechanics fluid_properties chemical_reactions)
set(_dep_thermal_hydraulics navier_stokes fluid_properties heat_transfer rdg ray_tracing
                            solid_properties misc)
set(_dep_scalar_transport   chemical_reactions navier_stokes thermal_hydraulics
                            fluid_properties heat_transfer rdg ray_tracing solid_properties misc)
set(_dep_subchannel         fluid_properties heat_transfer reactor)
set(_dep_xfem               solid_mechanics)

# Module creation order: every module appears after all of its dependencies (matches the
# top-down ordering in modules.mk).
set(MOOSE_MODULES_ORDERED
  chemical_reactions electromagnetics external_petsc_solver functional_expansion_tools
  geochemistry level_set misc ray_tracing rdg reactor stochastic_tools
  fluid_properties heat_transfer solid_properties solid_mechanics navier_stokes
  contact fsi optimization peridynamics phase_field porous_flow thermal_hydraulics
  scalar_transport subchannel xfem)

# Per-module enable options (default ON == ALL_MODULES).
foreach(_m IN LISTS MOOSE_MODULES_ORDERED)
  string(TOUPPER "${_m}" _M)
  option(MOOSE_MODULE_${_M} "Build the ${_m} module" ON)
endforeach()

# Auto-enable dependencies: walk in REVERSE order so enabling a depender flips on its deps
# before we reach them. One reverse pass suffices because the list is topologically ordered.
set(_rev ${MOOSE_MODULES_ORDERED})
list(REVERSE _rev)
foreach(_m IN LISTS _rev)
  string(TOUPPER "${_m}" _M)
  if(MOOSE_MODULE_${_M})
    foreach(_d IN LISTS _dep_${_m})
      string(TOUPPER "${_d}" _D)
      if(NOT MOOSE_MODULE_${_D})
        message(STATUS "Module ${_m} requires ${_d}; enabling it.")
        set(MOOSE_MODULE_${_D} ON CACHE BOOL "Build the ${_d} module" FORCE)
      endif()
    endforeach()
  endif()
endforeach()

# Create the module library targets (+ per-module exec and test lib) in dependency order.
set(_enabled_modules "")
foreach(_m IN LISTS MOOSE_MODULES_ORDERED)
  string(TOUPPER "${_m}" _M)
  if(MOOSE_MODULE_${_M})
    moose_add_app(NAME ${_m}
                  APP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${_m}"
                  BUILD_EXEC WITH_TEST_LIB
                  DEPEND_MODULES ${_dep_${_m}})
    list(APPEND _enabled_modules ${_m})
  endif()
endforeach()
# Expose to the top-level scope (moose_install installs/exports these).
set(MOOSE_ENABLED_MODULES "${_enabled_modules}" CACHE INTERNAL "Enabled MOOSE module names")

# combined: built only when every module is enabled (mirrors modules.mk's ALL_MODULES guard).
# CombinedApp registers all modules (each guarded by <NAME>_ENABLED, inherited via the module
# libs) and CombinedTestApp registers every module's test objects -- so the combined test lib
# links every module's test library (the make build's USE_TEST_LIBS=yes).
list(LENGTH MOOSE_MODULES_ORDERED _n_all)
list(LENGTH _enabled_modules _n_on)
if(_n_on EQUAL _n_all)
  moose_add_app(NAME combined
                APP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/combined"
                BUILD_EXEC WITH_TEST_LIB GEN_REVISION
                DEPEND_MODULES ${MOOSE_MODULES_ORDERED})
  moose_method_libname(_combined_test combined_test)
  if(TARGET ${_combined_test})
    foreach(_m IN LISTS MOOSE_MODULES_ORDERED)
      target_link_libraries(${_combined_test} PUBLIC Moose::${_m}_test)
    endforeach()
  endif()
endif()

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose includes
#include "RhieChowMassFlux.h"
#include "SIMPLESolveBase.h"

/**
 * SIMPLE-based solution object with nonlinear FV system assembly.
 */
class SIMPLESolveNonlinearAssembly : public SIMPLESolveBase
{
public:
  SIMPLESolveNonlinearAssembly(Executioner & ex);

  static InputParameters validParams();

  /// Fetch the Rhie Chow user object that
  virtual void linkRhieChowUserObject() override;

  /**
   * Performs the momentum pressure coupling.
   * @return True if solver is converged.
   */
  virtual bool solve() override;

protected:
  virtual std::vector<std::pair<unsigned int, Real>> solveMomentumPredictor() override;
  virtual std::pair<unsigned int, Real> solvePressureCorrector() override;

  /// The number(s) of the system(s) corresponding to the momentum equation(s)
  std::vector<unsigned int> _momentum_system_numbers;

  /// Pointer(s) to the system(s) corresponding to the momentum equation(s)
  std::vector<NonlinearSystemBase *> _momentum_systems;

  /// The number of the system corresponding to the pressure equation
  const unsigned int _pressure_sys_number;

  /// Reference to the nonlinear system corresponding to the pressure equation
  NonlinearSystemBase & _pressure_system;

  // ************************ For quick equation check ********************* //

  /// Boolean for easy check if a fluid energy system shall be solved or not
  const bool _has_energy_system;

  /// Boolean for easy check if a solid energy system shall be solved or not
  const bool _has_solid_energy_system;

  /// Boolean for easy check if a passive scalar systems shall be solved or not
  const bool _has_passive_scalar_systems;

  /// Boolean for easy check if turbulence systems shall be solved or not
  const bool _has_turbulence_systems;

  // ************************ Energy Eq Variables ************************** //

  /// The number of the system corresponding to the energy equation
  const unsigned int _energy_sys_number;

  /// Pointer to the nonlinear system corresponding to the fluid energy equation
  NonlinearSystemBase * _energy_system;

  /// The user-defined relaxation parameter for the energy equation
  const Real _energy_equation_relaxation;

  /// Options which hold the petsc settings for the fluid energy equation
  Moose::PetscSupport::PetscOptions _energy_petsc_options;

  /// Options for the linear solver of the energy equation
  SIMPLESolverConfiguration _energy_linear_control;

  /// Absolute linear tolerance for the energy equations. We need to store this, because
  /// it needs to be scaled with a representative flux.
  const Real _energy_l_abs_tol;

  // ********************* Solid Energy Eq Variables *********************** //

  /// The number of the system corresponding to the solid energy equation
  const unsigned int _solid_energy_sys_number;

  /// Pointer to the nonlinear system corresponding to the solid energy equation
  NonlinearSystemBase * _solid_energy_system;

  /// Options which hold the petsc settings for the solid energy equation
  Moose::PetscSupport::PetscOptions _solid_energy_petsc_options;

  /// Options for the linear solver of the solid energy equation
  SIMPLESolverConfiguration _solid_energy_linear_control;

  /// Absolute linear tolerance for the solid energy equations. We need to store this, because
  /// it needs to be scaled with a representative flux.
  const Real _solid_energy_l_abs_tol;

  // ******************* Passive scalar Eq Variables *********************** //

  /// The names of the passive scalar systems
  const std::vector<SolverSystemName> & _passive_scalar_system_names;

  // The number(s) of the system(s) corresponding to the passive scalar equation(s)
  std::vector<unsigned int> _passive_scalar_system_numbers;

  /// Pointer(s) to the system(s) corresponding to the passive scalar equation(s)
  std::vector<NonlinearSystemBase *> _passive_scalar_systems;

  /// The user-defined relaxation parameter(s) for the passive scalar equation(s)
  const std::vector<Real> _passive_scalar_equation_relaxation;

  /// Options which hold the petsc settings for the passive scalar equation(s)
  Moose::PetscSupport::PetscOptions _passive_scalar_petsc_options;

  /// Options for the linear solver of the passive scalar equation(s)
  SIMPLESolverConfiguration _passive_scalar_linear_control;

  /// Absolute linear tolerance for the passive scalar equation(s). We need to store this, because
  /// it needs to be scaled with a representative flux.
  const Real _passive_scalar_l_abs_tol;

  // ********************* Turbulence Eq Variables ************************* //

  /// The names of the turbulence scalar systems
  const std::vector<SolverSystemName> & _turbulence_system_names;

  // The number(s) of the system(s) corresponding to the turbulence equation(s)
  std::vector<unsigned int> _turbulence_system_numbers;

  /// Pointer(s) to the system(s) corresponding to the turbulence equation(s)
  std::vector<NonlinearSystemBase *> _turbulence_systems;

  /// The user-defined relaxation parameter(s) for the turbulence equation(s)
  const std::vector<Real> _turbulence_equation_relaxation;

  /// The user-defined lower limit for turbulent quantities e.g. k, eps/omega, etc..
  std::vector<Real> _turbulence_field_min_limit;

  /// Options which hold the petsc settings for the turbulence equation(s)
  Moose::PetscSupport::PetscOptions _turbulence_petsc_options;

  /// Options for the linear solver of the turbulence equation(s)
  SIMPLESolverConfiguration _turbulence_linear_control;

  /// Absolute linear tolerance for the turbulence equation(s). We need to store this, because
  /// it needs to be scaled with a representative flux.
  const Real _turbulence_l_abs_tol;

  // ********************* SIMPLE iteration variables ********************** //

  /// The user-defined absolute tolerance for determining the convergence in energy
  const Real _energy_absolute_tolerance;

  /// The user-defined absolute tolerance for determining the convergence in solid energy
  const Real _solid_energy_absolute_tolerance;

  /// The user-defined absolute tolerance for determining the convergence in passive scalars
  const std::vector<Real> _passive_scalar_absolute_tolerance;

  /// The user-defined absolute tolerance for determining the convergence in turbulence equations
  const std::vector<Real> _turbulence_absolute_tolerance;

  // *************************** Other variables *************************** //

  /// Pointer to the segregated RhieChow interpolation object
  INSFVRhieChowInterpolatorSegregated * _rc_uo;

  /// The name of the vector tag which corresponds to the pressure gradient terms in the
  /// momentum equation
  const TagName _pressure_tag_name;

  /// The ID of the tag which corresponds to the pressure gradient terms in the
  /// momentum equation
  const TagID _pressure_tag_id;
};

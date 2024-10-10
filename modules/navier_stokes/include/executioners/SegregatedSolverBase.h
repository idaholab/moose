//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Executioner.h"
#include "INSFVRhieChowInterpolatorSegregated.h"
#include "PetscSupport.h"
#include "SolverParams.h"
#include "SegregatedSolverUtils.h"

#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_matrix.h"
#include "libmesh/equation_systems.h"
#include "libmesh/solver_configuration.h"

// Forward declarations
class InputParameters;
class FEProblemBase;

/**
 * Solver configuration class used with the linear solvers in a SIMPLE solver.
 */
class SIMPLESolverConfiguration : public libMesh::SolverConfiguration
{
  /**
   * Override this to make sure the PETSc options are not overwritten in the linear solver
   */
  virtual void configure_solver() override {}
};

/**
 * Base class for the executioners relying on segregated solution approaches.
 */
class SegregatedSolverBase : public Executioner
{
public:
  static InputParameters validParams();

  SegregatedSolverBase(const InputParameters & parameters);

  virtual void init() override;
  virtual bool lastSolveConverged() const override { return _last_solve_converged; }

protected:
  /**
   * Determine if the iterative process on a set of equations converged or not
   * @param residuals The current (linear iteration number, residual) pairs for the systems.
   * @param abs_tolerances The corresponding absolute tolerances.
   */
  bool converged(const std::vector<std::pair<unsigned int, Real>> & residuals,
                 const std::vector<Real> & abs_tolerances);

  void checkDependentParameterError(const std::string & main_parameter,
                                    const std::vector<std::string> & dependent_parameters,
                                    const bool should_be_defined);

  /// Check for wrong execute on flags in the multiapps
  bool hasMultiAppError(const ExecFlagEnum & flags);

  /// Check for wrong execute on flags in the transfers
  bool hasTransferError(const ExecFlagEnum & flags);

  /// Reference to the MOOSE Problem which contains this executioner
  FEProblemBase & _problem;

  /// Time-related member variables. Only used to set the steady-state result
  /// at time 1.0;
  Real _system_time;
  int & _time_step;
  Real & _time;

  bool _last_solve_converged;

  /// Boolean for easy check if a fluid energy system shall be solved or not
  const bool _has_energy_system;

  /// Boolean for easy check if a solid energy system shall be solved or not
  const bool _has_solid_energy_system;

  /// Boolean for easy check if a passive scalar systems shall be solved or not
  const bool _has_passive_scalar_systems;

  /// Boolean for easy check if turbulence systems shall be solved or not
  const bool _has_turbulence_systems;

  /// The names of the momentum systems.
  const std::vector<SolverSystemName> & _momentum_system_names;

  /// The names of the passive scalar systems
  const std::vector<SolverSystemName> & _passive_scalar_system_names;

  /// The names of the turbulence scalar systems
  const std::vector<SolverSystemName> & _turbulence_system_names;

  /// The user-defined relaxation parameter for the momentum equation
  const Real _momentum_equation_relaxation;

  /// The user-defined relaxation parameter for the pressure variable
  const Real _pressure_variable_relaxation;

  /// The user-defined relaxation parameter for the energy equation
  const Real _energy_equation_relaxation;

  /// The user-defined relaxation parameter(s) for the passive scalar equation(s)
  const std::vector<Real> _passive_scalar_equation_relaxation;

  /// The user-defined relaxation parameter(s) for the turbulence equation(s)
  const std::vector<Real> _turbulence_equation_relaxation;

  /// The user-defined lower limit for turbulent quantities e.g. k, eps/omega, etc..
  std::vector<Real> _turbulence_field_min_limit;

  /// The user-defined absolute tolerance for determining the convergence in momentum
  const Real _momentum_absolute_tolerance;

  /// The user-defined absolute tolerance for determining the convergence in pressure
  const Real _pressure_absolute_tolerance;

  /// The user-defined absolute tolerance for determining the convergence in energy
  const Real _energy_absolute_tolerance;

  /// The user-defined absolute tolerance for determining the convergence in solid energy
  const Real _solid_energy_absolute_tolerance;

  /// The user-defined absolute tolerance for determining the convergence in passive scalars
  const std::vector<Real> _passive_scalar_absolute_tolerance;

  /// The user-defined absolute tolerance for determining the convergence in turbulence equations
  const std::vector<Real> _turbulence_absolute_tolerance;

  /// The maximum number of momentum-pressure iterations
  const unsigned int _num_iterations;

  /// Debug parameter which allows printing the coupling and solution vectors/matrices
  const bool _print_fields;

  /// Options which hold the petsc settings for the momentum equation
  Moose::PetscSupport::PetscOptions _momentum_petsc_options;

  /// Options which hold the petsc settings for the pressure equation
  Moose::PetscSupport::PetscOptions _pressure_petsc_options;

  /// Options which hold the petsc settings for the fluid energy equation
  Moose::PetscSupport::PetscOptions _energy_petsc_options;

  /// Options which hold the petsc settings for the solid energy equation
  Moose::PetscSupport::PetscOptions _solid_energy_petsc_options;

  /// Options which hold the petsc settings for the passive scalar equation(s)
  Moose::PetscSupport::PetscOptions _passive_scalar_petsc_options;

  /// Options which hold the petsc settings for the turbulence equation(s)
  Moose::PetscSupport::PetscOptions _turbulence_petsc_options;

  /// Options for the linear solver of the momentum equation
  SIMPLESolverConfiguration _momentum_linear_control;

  /// Absolute linear tolerance for the momentum equation(s). We need to store this, because
  /// it needs to be scaled with a representative flux.
  const Real _momentum_l_abs_tol;

  /// Options for the linear solver of the pressure equation
  SIMPLESolverConfiguration _pressure_linear_control;

  /// Absolute linear tolerance for the pressure equation. We need to store this, because
  /// it needs to be scaled with a representative flux.
  const Real _pressure_l_abs_tol;

  /// Options for the linear solver of the energy equation
  SIMPLESolverConfiguration _energy_linear_control;

  /// Absolute linear tolerance for the energy equations. We need to store this, because
  /// it needs to be scaled with a representative flux.
  const Real _energy_l_abs_tol;

  /// Options for the linear solver of the solid energy equation
  SIMPLESolverConfiguration _solid_energy_linear_control;

  /// Absolute linear tolerance for the solid energy equations. We need to store this, because
  /// it needs to be scaled with a representative flux.
  const Real _solid_energy_l_abs_tol;

  /// Options for the linear solver of the passive scalar equation(s)
  SIMPLESolverConfiguration _passive_scalar_linear_control;

  /// Absolute linear tolerance for the passive scalar equation(s). We need to store this, because
  /// it needs to be scaled with a representative flux.
  const Real _passive_scalar_l_abs_tol;

  /// Options for the linear solver of the turbulence equation(s)
  SIMPLESolverConfiguration _turbulence_linear_control;

  /// Absolute linear tolerance for the turbulence equation(s). We need to store this, because
  /// it needs to be scaled with a representative flux.
  const Real _turbulence_l_abs_tol;

  /// If the pressure needs to be pinned
  const bool _pin_pressure;

  /// The value we want to enforce for pressure
  const Real _pressure_pin_value;

  /// The dof ID where the pressure needs to be pinned
  dof_id_type _pressure_pin_dof;
};

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
#include "SolveObject.h"
#include "UserObjectInterface.h"
#include "PetscSupport.h"
#include "SolverParams.h"
#include "SegregatedSolverUtils.h"

// Libmesh includes
#include "libmesh/solver_configuration.h"
#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_matrix.h"
#include "libmesh/equation_systems.h"

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
 * Solve class serving as a base class for the two SIMPLE solvers that operate with
 * different assembly algorithms. Includes base routines and variables for the coupling of
 * momentum and pressure.
 */
class SIMPLESolveBase : public SolveObject, public UserObjectInterface
{
public:
  SIMPLESolveBase(Executioner & ex);

  static InputParameters validParams();

  virtual void setInnerSolve(SolveObject &) override
  {
    mooseError("Cannot set inner solve object for solves that inherit from SIMPLESolveBase");
  }

  /// Fetch the Rhie Chow user object that is reponsible for determining face
  /// velocities and mass flux
  virtual void linkRhieChowUserObject() = 0;

  /// Setup pressure pin if there is need for one
  void setupPressurePin();

  /// Check if the user defined time kernels
  virtual void checkIntegrity() {}

protected:
  void checkDependentParameterError(const std::string & main_parameter,
                                    const std::vector<std::string> & dependent_parameters,
                                    const bool should_be_defined);

  /// Solve a momentum predictor step with a fixed pressure field
  /// @return A vector of (number of linear iterations, normalized residual norm) pairs for
  /// the momentum equations. The length of the vector equals the dimensionality of
  /// the domain.
  virtual std::vector<std::pair<unsigned int, Real>> solveMomentumPredictor() = 0;

  /// Solve a pressure corrector step.
  /// @return The number of linear iterations and the normalized residual norm of
  /// the pressure equation.
  virtual std::pair<unsigned int, Real> solvePressureCorrector() = 0;

  // ************************ Momentum Eq Variables ************************ //

  /// The names of the momentum systems.
  const std::vector<SolverSystemName> & _momentum_system_names;

  /// Options for the linear solver of the momentum equation
  SIMPLESolverConfiguration _momentum_linear_control;

  /// Absolute linear tolerance for the momentum equation(s). We need to store this, because
  /// it needs to be scaled with a representative flux.
  const Real _momentum_l_abs_tol;

  /// Options which hold the petsc settings for the momentum equation
  Moose::PetscSupport::PetscOptions _momentum_petsc_options;

  /// The user-defined relaxation parameter for the momentum equation
  const Real _momentum_equation_relaxation;

  // ************************ Pressure Eq Variables ************************ //

  /// The name of the pressure system
  const SolverSystemName & _pressure_system_name;

  /// Options for the linear solver of the pressure equation
  SIMPLESolverConfiguration _pressure_linear_control;

  /// Absolute linear tolerance for the pressure equation. We need to store this, because
  /// it needs to be scaled with a representative flux.
  const Real _pressure_l_abs_tol;

  /// Options which hold the petsc settings for the pressure equation
  Moose::PetscSupport::PetscOptions _pressure_petsc_options;

  /// The user-defined relaxation parameter for the pressure variable
  const Real _pressure_variable_relaxation;

  /// If the pressure needs to be pinned
  const bool _pin_pressure;

  /// The value we want to enforce for pressure
  const Real _pressure_pin_value;

  /// The dof ID where the pressure needs to be pinned
  dof_id_type _pressure_pin_dof;

  // ************************ Energy Eq Variables ************************** //

  /// Boolean for easy check if a fluid energy system shall be solved or not
  const bool _has_energy_system;

  /// The user-defined relaxation parameter for the energy equation
  const Real _energy_equation_relaxation;

  /// Options which hold the petsc settings for the fluid energy equation
  Moose::PetscSupport::PetscOptions _energy_petsc_options;

  /// Options for the linear solver of the energy equation
  SIMPLESolverConfiguration _energy_linear_control;

  /// Absolute linear tolerance for the energy equations. We need to store this, because
  /// it needs to be scaled with a representative flux.
  const Real _energy_l_abs_tol;

  // ************************ Passive Scalar Variables ************************ //

  /// The names of the passive scalar systems
  const std::vector<SolverSystemName> & _passive_scalar_system_names;

  /// Boolean for easy check if a passive scalar systems shall be solved or not
  const bool _has_passive_scalar_systems;

  // The number(s) of the system(s) corresponding to the passive scalar equation(s)
  std::vector<unsigned int> _passive_scalar_system_numbers;

  /// The user-defined relaxation parameter(s) for the passive scalar equation(s)
  const std::vector<Real> _passive_scalar_equation_relaxation;

  /// Options which hold the petsc settings for the passive scalar equation(s)
  Moose::PetscSupport::PetscOptions _passive_scalar_petsc_options;

  /// Options for the linear solver of the passive scalar equation(s)
  SIMPLESolverConfiguration _passive_scalar_linear_control;

  /// Absolute linear tolerance for the passive scalar equation(s). We need to store this, because
  /// it needs to be scaled with a representative flux.
  const Real _passive_scalar_l_abs_tol;

  // ************************ Iteration control **************************** //

  /// The user-defined absolute tolerance for determining the convergence in momentum
  const Real _momentum_absolute_tolerance;

  /// The user-defined absolute tolerance for determining the convergence in pressure
  const Real _pressure_absolute_tolerance;

  /// The user-defined absolute tolerance for determining the convergence in energy
  const Real _energy_absolute_tolerance;

  /// The user-defined absolute tolerance for determining the convergence in passive scalars
  const std::vector<Real> _passive_scalar_absolute_tolerance;

  /// The maximum number of momentum-pressure iterations
  const unsigned int _num_iterations;

  /// If solve should continue if maximum number of iterations is hit
  const bool _continue_on_max_its;

  // ************************ Other Variables ****************************** //

  /// Debug parameter which allows printing the coupling and solution vectors/matrices
  const bool _print_fields;
};

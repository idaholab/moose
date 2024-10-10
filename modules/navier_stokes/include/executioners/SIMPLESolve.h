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
#include "RhieChowMassFlux.h"
#include "SegregatedSolverBase.h"

class SIMPLESolve : public SolveObject
{
public:
  SIMPLESolve(Executioner & ex);

  static InputParameters validParams();

  /**
   * Will be the inner solve, so the SIMPLE solve.
   * @return True if solver is converged.
   */
  virtual bool solve() override;

  virtual void setInnerSolve(SolveObject &) override
  {
    mooseError("Cannot set inner solve for SIMPLESolve");
  }

  /// Fetch the Rhie Chow user object that
  void linkRhieChowUserObject();

  /// Setup pressure pin if there is need for one
  void setupPressurePin();

  /// Get the user object responsible for the Rhie-Chow interpolation
  const RhieChowMassFlux & getRCUserObject() { return *_rc_uo; }

protected:
  /// Solve a momentum predictor step with a fixed pressure field
  /// @return A vector of (number of linear iterations, normalized residual norm) pairs for
  /// the momentum equations. The length of the vector equals the dimensionality of
  /// the domain.
  std::vector<std::pair<unsigned int, Real>> solveMomentumPredictor();

  /// Solve a pressure corrector step.
  /// @return The number of linear iterations and the normalized residual norm of
  /// the pressure equation.
  std::pair<unsigned int, Real> solvePressureCorrector();

  /// The names of the momentum systems.
  const std::vector<SolverSystemName> & _momentum_system_names;

  /// The number(s) of the system(s) corresponding to the momentum equation(s)
  std::vector<unsigned int> _momentum_system_numbers;

  /// Pointer(s) to the system(s) corresponding to the momentum equation(s)
  std::vector<LinearSystem *> _momentum_systems;

  /// The name of the pressure system
  const SolverSystemName & _pressure_system_name;

  /// The number of the system corresponding to the pressure equation
  const unsigned int _pressure_sys_number;

  /// Reference to the nonlinear system corresponding to the pressure equation
  LinearSystem & _pressure_system;

  /// Pointer to the segregated RhieChow interpolation object
  RhieChowMassFlux * _rc_uo;

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

  /// The user-defined absolute tolerance for determining the convergence in momentum
  const Real _momentum_absolute_tolerance;

  /// The user-defined absolute tolerance for determining the convergence in pressure
  const Real _pressure_absolute_tolerance;

  /// Options which hold the petsc settings for the momentum equation
  Moose::PetscSupport::PetscOptions _momentum_petsc_options;

  /// Options which hold the petsc settings for the pressure equation
  Moose::PetscSupport::PetscOptions _pressure_petsc_options;

  /// The user-defined relaxation parameter for the momentum equation
  const Real _momentum_equation_relaxation;

  /// The user-defined relaxation parameter for the pressure variable
  const Real _pressure_variable_relaxation;

  /// If the pressure needs to be pinned
  const bool _pin_pressure;

  /// The value we want to enforce for pressure
  const Real _pressure_pin_value;

  /// The dof ID where the pressure needs to be pinned
  dof_id_type _pressure_pin_dof;

  /// Debug parameter which allows printing the coupling and solution vectors/matrices
  const bool _print_fields;
};

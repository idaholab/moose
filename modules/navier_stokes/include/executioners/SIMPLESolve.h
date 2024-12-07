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
 * SIMPLE-based solution object with linear FV system assembly.
 */
class SIMPLESolve : public SIMPLESolveBase
{
public:
  SIMPLESolve(Executioner & ex);

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

  /// Solve an equation which contains an advection term that depends
  /// on the solution of the segregated Navier-Stokes equations.
  /// @param system_num The number of the system which is solved
  /// @param system Reference to the system which is solved
  /// @param relaxation_factor The relaxation factor for matrix relaxation
  /// @param solver_config The solver configuration object for the linear solve
  /// @param abs_tol The scaled absolute tolerance for the linear solve
  /// @return The normalized residual norm of the equation.
  std::pair<unsigned int, Real> solveAdvectedSystem(const unsigned int system_num,
                                                    LinearSystem & system,
                                                    const Real relaxation_factor,
                                                    libMesh::SolverConfiguration & solver_config,
                                                    const Real abs_tol);

  /// The number(s) of the system(s) corresponding to the momentum equation(s)
  std::vector<unsigned int> _momentum_system_numbers;

  /// Pointer(s) to the system(s) corresponding to the momentum equation(s)
  std::vector<LinearSystem *> _momentum_systems;

  /// The number of the system corresponding to the pressure equation
  const unsigned int _pressure_sys_number;

  /// Reference to the nonlinear system corresponding to the pressure equation
  LinearSystem & _pressure_system;

  /// The number of the system corresponding to the energy equation
  const unsigned int _energy_sys_number;

  /// Pointer to the nonlinear system corresponding to the fluid energy equation
  LinearSystem * _energy_system;

  /// Pointer(s) to the system(s) corresponding to the passive scalar equation(s)
  std::vector<LinearSystem *> _passive_scalar_systems;

  /// Pointer to the segregated RhieChow interpolation object
  RhieChowMassFlux * _rc_uo;
};

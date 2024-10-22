//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SegregatedSolverBase.h"
#include "SIMPLESolveNonlinearAssembly.h"

// Forward declarations
class InputParameters;
class FEProblemBase;

/**
 * Executioner set up to solve a thermal-hydraulics problem using the
 * SIMPLENonlinearAssembly algorithm. It utilizes segregated linearized systems
 * which are solved using a fixed-point iteration.
 */
class SIMPLENonlinearAssembly : public SegregatedSolverBase
{
public:
  static InputParameters validParams();

  SIMPLENonlinearAssembly(const InputParameters & parameters);

  virtual void init() override;
  virtual void execute() override;
  virtual bool lastSolveConverged() const override { return _last_solve_converged; }

protected:
  /// Solve a momentum predictor step with a fixed pressure field
  /// @return A vector for the normalized residual norms of the momentum equations.
  ///         The length of the vector equals the dimensionality of the domain.
  std::vector<std::pair<unsigned int, Real>> solveMomentumPredictor();

  /// Solve a pressure corrector step.
  /// @return The normalized residual norm of the pressure equation.
  std::pair<unsigned int, Real> solvePressureCorrector();

  SIMPLESolveNonlinearAssembly _simple_solve;
};

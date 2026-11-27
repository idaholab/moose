//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Executor.h"

class SolverSystem;

/**
 * Solves one solver system, nonlinear or linear. The PETSc options, the linear solver tolerances
 * and the Convergence object of this executor apply to its system only.
 */
class SystemSolveExecutor : public Executor
{
public:
  static InputParameters validParams();

  SystemSolveExecutor(const InputParameters & parameters);

  virtual void initialSetup() override;

protected:
  virtual Result run() override;

  /// The system that this executor solves
  SolverSystem & _sys;
  /// Whether the system is a nonlinear system
  const bool _is_nonlinear;
  /// The fraction of the new solution that a converged solve keeps, see SolverSystem::applyFixedPointRelaxation
  const Real _relaxation_factor;
  /// The label of the convergence status line
  const std::string _solve_name;
};

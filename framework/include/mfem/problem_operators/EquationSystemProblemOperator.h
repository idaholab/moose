//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "ProblemOperator.h"
#include "EquationSystemInterface.h"

namespace Moose::MFEM
{
/**
 * Steady-state ProblemOperator that assembles and solves a single EquationSystem each step.
 *
 * On each call to Solve() this class:
 *   1. Rebuilds the equation-system operator (kernels → forms → assembled matrix).
 *   2. Calls EquationSystem::prepareLinearSolver() to propagate the assembled operator
 *      (and the bilinear form for LOR preconditioners) to the configured solver tree.
 *   3. Dispatches to the linear or nonlinear solve path via SolveWithOperator().
 *   4. Scatters the true-DoF solution back to the grid functions.
 *
 * @see EquationSystem for the class that owns the weak-form mathematics.
 * @see ProblemOperatorBase for the block-vector bookkeeping and solve-dispatch infrastructure.
 */
class EquationSystemProblemOperator : public ProblemOperator, public EquationSystemInterface
{
public:
  EquationSystemProblemOperator(Problem & problem)
    : ProblemOperator(problem), _equation_system(_problem_data.eqn_system)
  {
  }

  virtual void SetGridFunctions() override;
  virtual void Solve() override;

  [[nodiscard]] virtual EquationSystem * GetEquationSystem() const override
  {
    mooseAssert(_equation_system, "No EquationSystem in EquationSystemProblemOperator.");
    return _equation_system.get();
  }

protected:
  /// Add kernels/bcs and assemble the linear part of the equation system
  void BuildEquationSystemOperator();

private:
  std::shared_ptr<EquationSystem> _equation_system{nullptr};
};

} // namespace Moose::MFEM

#endif

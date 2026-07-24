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
 * Steady-state ProblemOperator that prepares and solves a single EquationSystem.
 *
 * On each call to Solve() this class:
 *   1. Forms equation-system data (kernels -> weak forms -> constrained linear part and
 *      nonlinear action forms).
 *   2. Calls EquationSystem::ProvideOperator() to pass either the EquationSystem itself or
 *      the assembled linear operator (and the bilinear form for LOR preconditioners) to the
 *      configured solver tree.
 *   3. Dispatches to the linear or nonlinear solve path via SolveWithOperator().
 *   4. Scatters the true-DoF solution back to the grid functions.
 *
 * @see EquationSystem for the class that owns the weak-form mathematics.
 * @see ProblemOperatorBase for the block-vector bookkeeping and solve-dispatch infrastructure.
 */
class EquationSystemProblemOperator : public ProblemOperator, public EquationSystemInterface
{
public:
  EquationSystemProblemOperator(MFEMProblem & problem)
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
  /// Form equation-system state used by Solve().
  void FormEquationSystemOperator();

private:
  std::shared_ptr<EquationSystem> _equation_system{nullptr};
};

} // namespace Moose::MFEM

#endif

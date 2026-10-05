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

#include "MFEMSolverBase.h"
#include "EquationSystem.h"

class MFEMProblemSolve;

namespace Moose::MFEM
{
/**
 * Base class for linear MFEM solvers and preconditioners.
 */
class LinearSolverBase : public SolverBase
{
public:
  static InputParameters validParams();

  LinearSolverBase(const InputParameters & parameters);

  /// Returns this solver's preconditioner
  LinearSolverBase * GetPreconditioner() { return _preconditioner.get(); }

  virtual void SetOperator(const mfem::Operator & op) override;

  /// For eigensolvers, this method calls the underlying Solve method
  virtual void Solve() { mooseError("'solve' method not used in this solver type."); }

protected:
  /// Called once per SetOperator, only when a preconditioner is set.
  /// Override in solvers that accept a preconditioner.
  virtual void SetPreconditionerImpl() { mooseError("This solver does not support a preconditioner."); }

  /// Preconditioner to be used for the problem
  std::shared_ptr<LinearSolverBase> _preconditioner;

  /// Pointer to EquationSystem used for problem-specific solver setup
  std::shared_ptr<EquationSystem> _equation_system;

private:
  friend class ::MFEMProblemSolve;
};
} // namespace Moose::MFEM

#endif

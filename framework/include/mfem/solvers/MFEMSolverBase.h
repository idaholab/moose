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

#include "MFEMObject.h"

namespace Moose::MFEM
{
/**
 * Base class for wrapping mfem::Solver-derived classes.
 */
class SolverBase : public MFEMObject
{
public:
  static InputParameters validParams();

  SolverBase(const InputParameters & parameters);

  /// Returns the wrapped MFEM solver
  mfem::Solver & GetSolver();

  /// Override in derived classes to construct and set the solver options.
  virtual void ConstructSolver() = 0;

  /// Set the operator for the solver
  virtual void SetOperator(mfem::Operator & op) = 0;

protected:
  /// Solver to be used for the problem
  std::unique_ptr<mfem::Solver> _solver;
};

inline mfem::Solver &
SolverBase::GetSolver()
{
  mooseAssert(_solver, "Attempting to retrieve solver before it's been constructed");
  return *_solver;
}
} // namespace Moose::MFEM

#endif

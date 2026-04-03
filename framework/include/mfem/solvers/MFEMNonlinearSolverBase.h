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

#include "libmesh/ignore_warnings.h"
#include "mfem.hpp"
#include "libmesh/restore_warnings.h"

namespace Moose::MFEM
{
/**
 * MooseObject base for nonlinear MFEM solve strategies configured in the input file.
 */
class NonlinearSolverBase : public SolverBase
{
public:
  virtual ~NonlinearSolverBase() = default;

  static InputParameters validParams();

  NonlinearSolverBase(const InputParameters & parameters);

  void constructSolver() override = 0;

  /// Configure the nonlinear solver with the residual/Jacobian operator.
  virtual void SetOperator(const mfem::Operator & op) = 0;

  /// Configure the linear solver used inside the nonlinear solve.
  virtual void SetLinearSolver(mfem::Solver & solver) = 0;

  /// Solve the nonlinear system for the provided right-hand side and solution vector.
  virtual void Mult(const mfem::Vector & rhs, mfem::Vector & x) = 0;

  /// Return whether this nonlinear solver requires Jacobian/gradient information from the operator.
  virtual bool requiresGradient() const = 0;

  /// Return whether this nonlinear solver uses the externally configured MFEM linear solver.
  virtual bool usesExternalLinearSolver() const = 0;
};
} // namespace Moose::MFEM

#endif

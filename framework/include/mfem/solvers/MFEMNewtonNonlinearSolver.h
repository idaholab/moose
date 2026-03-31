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

#include "MFEMNonlinearSolverBase.h"

namespace Moose::MFEM
{
/**
 * Nonlinear solver wrapper backed by mfem::NewtonSolver.
 */
class NewtonNonlinearSolver : public NonlinearSolverBase
{
public:
  NewtonNonlinearSolver(MPI_Comm comm,
                        unsigned int max_its,
                        mfem::real_t abs_tol,
                        mfem::real_t rel_tol,
                        unsigned int print_level);

  void SetOperator(const mfem::Operator & op) override;
  void SetPreconditioner(mfem::Solver & solver) override;
  void Mult(const mfem::Vector & rhs, mfem::Vector & x) override;
  bool requiresGradient() const override { return true; }

  /// Access the wrapped MFEM Newton solver.
  mfem::NewtonSolver & getSolver() { return _solver; }

private:
  mfem::NewtonSolver _solver;
};
} // namespace Moose::MFEM

#endif

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
class EquationSystem;

/**
 * Residual-only Picard solver using the equation system linear operator and explicit nonlinear
 * residual evaluation.
 */
class PicardNonlinearSolver : public NonlinearSolverBase
{
public:
  PicardNonlinearSolver(MPI_Comm comm,
                        unsigned int max_its,
                        mfem::real_t abs_tol,
                        mfem::real_t rel_tol,
                        unsigned int print_level,
                        mfem::real_t damping);

  void SetOperator(const mfem::Operator & op) override;
  void SetPreconditioner(mfem::Solver & solver) override;
  void Mult(const mfem::Vector & rhs, mfem::Vector & x) override;
  bool requiresGradient() const override { return false; }

private:
  const mfem::Operator * _op{nullptr};
  const Moose::MFEM::EquationSystem * _equation_system{nullptr};
  mfem::real_t _abs_tol;
  mfem::real_t _rel_tol;
  unsigned int _max_its;
  unsigned int _print_level;
  mfem::real_t _damping;
  MPI_Comm _comm;
};
} // namespace Moose::MFEM

#endif

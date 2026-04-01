//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMNewtonNonlinearSolver.h"

namespace Moose::MFEM
{
NewtonNonlinearSolver::NewtonNonlinearSolver(MPI_Comm comm,
                                             unsigned int max_its,
                                             mfem::real_t abs_tol,
                                             mfem::real_t rel_tol,
                                             unsigned int print_level,
                                             bool use_initial_guess)
  : _solver(comm)
{
  _solver.iterative_mode = use_initial_guess;
  _solver.SetRelTol(rel_tol);
  _solver.SetAbsTol(abs_tol);
  _solver.SetMaxIter(max_its);
  _solver.SetPrintLevel(print_level);
}

void
NewtonNonlinearSolver::SetOperator(const mfem::Operator & op)
{
  _solver.SetOperator(op);
}

void
NewtonNonlinearSolver::SetPreconditioner(mfem::Solver & solver)
{
  _solver.SetSolver(solver);
}

void
NewtonNonlinearSolver::Mult(const mfem::Vector & rhs, mfem::Vector & x)
{
  _solver.Mult(rhs, x);
}
} // namespace Moose::MFEM

#endif

//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMPetscNonlinearSolver.h"
#include "MooseError.h"

#ifdef MFEM_USE_PETSC

namespace Moose::MFEM
{
PetscNonlinearSolver::PetscNonlinearSolver(MPI_Comm comm,
                                           unsigned int max_its,
                                           mfem::real_t abs_tol,
                                           mfem::real_t rel_tol,
                                           unsigned int print_level,
                                           const std::string & options_prefix,
                                           bool use_initial_guess)
  : _solver(comm,
            !options_prefix.empty() && options_prefix.back() != '_'
                ? options_prefix + "_"
                : options_prefix)
{
  _solver.iterative_mode = use_initial_guess;
  _solver.SetRelTol(rel_tol);
  _solver.SetAbsTol(abs_tol);
  _solver.SetMaxIter(max_its);
  _solver.SetPrintLevel(print_level);
  _solver.SetJacobianType(mfem::Operator::PETSC_MATAIJ);
}

void
PetscNonlinearSolver::SetOperator(const mfem::Operator & op)
{
  _solver.SetOperator(op);
}

void
PetscNonlinearSolver::SetPreconditioner(mfem::Solver &)
{
}

void
PetscNonlinearSolver::Mult(const mfem::Vector & rhs, mfem::Vector & x)
{
  _solver.Mult(rhs, x);
}
} // namespace Moose::MFEM

#endif

#endif

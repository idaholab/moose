//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMPicardNonlinearSolver.h"
#include "EquationSystem.h"
#include "MooseError.h"

namespace Moose::MFEM
{
PicardNonlinearSolver::PicardNonlinearSolver(MPI_Comm comm,
                                             unsigned int max_its,
                                             mfem::real_t abs_tol,
                                             mfem::real_t rel_tol,
                                             unsigned int print_level,
                                             mfem::real_t damping)
  : _abs_tol(abs_tol),
    _rel_tol(rel_tol),
    _max_its(max_its),
    _print_level(print_level),
    _damping(damping),
    _comm(comm)
{
}

void
PicardNonlinearSolver::SetOperator(const mfem::Operator & op)
{
  _op = &op;
  _equation_system = dynamic_cast<const Moose::MFEM::EquationSystem *>(&op);
  if (!_equation_system)
    mooseError("PicardNonlinearSolver requires a Moose::MFEM::EquationSystem operator.");
}

void
PicardNonlinearSolver::SetPreconditioner(mfem::Solver &)
{
}

void
PicardNonlinearSolver::Mult(const mfem::Vector & rhs, mfem::Vector & x)
{
  mooseAssert(_op, "PicardNonlinearSolver operator has not been set.");
  mooseAssert(_equation_system, "PicardNonlinearSolver equation system has not been set.");

  mfem::GMRESSolver linear_solver(_comm);
  linear_solver.SetRelTol(1.0e-12);
  linear_solver.SetAbsTol(0.0);
  linear_solver.SetMaxIter(200);
  linear_solver.SetPrintLevel(0);
  linear_solver.SetOperator(_equation_system->GetLinearOperator());

  mfem::Vector residual(rhs.Size()), nonlinear_residual(rhs.Size()), lagged_rhs(rhs.Size()),
      next_x(x.Size()), previous_x(x.Size());

  _op->Mult(x, residual);
  residual -= rhs;
  const auto initial_norm = residual.Norml2();
  if (initial_norm <= _abs_tol)
    return;

  for (unsigned int it = 0; it < _max_its; ++it)
  {
    _equation_system->ComputeNonlinearResidual(x, nonlinear_residual);
    lagged_rhs = rhs;
    lagged_rhs -= nonlinear_residual;

    next_x = 0.0;
    linear_solver.Mult(lagged_rhs, next_x);

    if (_damping != 1.0)
    {
      previous_x = x;
      x.Set(1.0 - _damping, previous_x);
      x.Add(_damping, next_x);
    }
    else
      x = next_x;

    _op->Mult(x, residual);
    residual -= rhs;
    const auto residual_norm = residual.Norml2();

    if (_print_level > 0)
      mfem::out << "Picard iteration " << it + 1 << ", |r| = " << residual_norm << '\n';

    if (residual_norm <= _abs_tol || residual_norm <= _rel_tol * initial_norm)
      return;
  }
}
} // namespace Moose::MFEM

#endif

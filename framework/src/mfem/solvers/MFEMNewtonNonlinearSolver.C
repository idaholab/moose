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
#include "MFEMProblem.h"

registerMooseMFEMObject("MooseApp", NewtonNonlinearSolver);

namespace Moose::MFEM
{
InputParameters
NewtonNonlinearSolver::validParams()
{
  InputParameters params = NonlinearSolverBase::validParams();
  params.addClassDescription("MFEM native nonlinear solver using Newton's method.");
  return params;
}

NewtonNonlinearSolver::NewtonNonlinearSolver(const InputParameters & parameters)
  : NonlinearSolverBase(parameters)
{
  constructSolver();
}

void
NewtonNonlinearSolver::constructSolver()
{
  auto solver = std::make_unique<mfem::NewtonSolver>(getMFEMProblem().getComm());
  solver->iterative_mode = getParam<bool>("use_initial_guess");
  solver->SetRelTol(getParam<mfem::real_t>("rel_tol"));
  solver->SetAbsTol(getParam<mfem::real_t>("abs_tol"));
  solver->SetMaxIter(getParam<unsigned int>("max_its"));
  solver->SetPrintLevel(getParam<unsigned int>("print_level"));
  _solver = std::move(solver);
}

void
NewtonNonlinearSolver::SetOperator(const mfem::Operator & op)
{
  static_cast<mfem::NewtonSolver &>(getSolver()).SetOperator(op);
}

void
NewtonNonlinearSolver::SetLinearSolver(mfem::Solver & solver)
{
  static_cast<mfem::NewtonSolver &>(getSolver()).SetSolver(solver);
}

void
NewtonNonlinearSolver::Mult(const mfem::Vector & rhs, mfem::Vector & x)
{
  getSolver().Mult(rhs, x);
}
} // namespace Moose::MFEM
#endif

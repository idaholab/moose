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

registerMooseObject("MooseApp", MFEMNewtonNonlinearSolver);

InputParameters
MFEMNewtonNonlinearSolver::validParams()
{
  InputParameters params = Moose::MFEM::NonlinearSolverBase::validParams();
  params.addClassDescription("MFEM native nonlinear solver using Newton's method.");
  return params;
}

MFEMNewtonNonlinearSolver::MFEMNewtonNonlinearSolver(const InputParameters & parameters)
  : Moose::MFEM::NonlinearSolverBase(parameters)
{
  constructSolver(parameters);
}

void
MFEMNewtonNonlinearSolver::constructSolver(const InputParameters & parameters)
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
MFEMNewtonNonlinearSolver::SetOperator(const mfem::Operator & op)
{
  static_cast<mfem::NewtonSolver &>(getSolver()).SetOperator(op);
}

void
MFEMNewtonNonlinearSolver::SetLinearSolver(mfem::Solver & solver)
{
  static_cast<mfem::NewtonSolver &>(getSolver()).SetSolver(solver);
}

void
MFEMNewtonNonlinearSolver::Mult(const mfem::Vector & rhs, mfem::Vector & x)
{
  getSolver().Mult(rhs, x);
}
#endif

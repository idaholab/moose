//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMCGSolver.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMCGSolver);

InputParameters
MFEMCGSolver::validParams()
{
  InputParameters params = Moose::MFEM::LORLinearSolverBase<mfem::CGSolver>::validParams();
  params.addClassDescription("MFEM native solver for the iterative solution of MFEM equation "
                             "systems using the conjugate gradient method.");
  params.set<bool>("use_initial_guess", /*quiet_mode=*/true) = true;
  params.addParam<mfem::real_t>("l_tol", 1e-5, "Set the relative tolerance.");
  params.addParam<mfem::real_t>("l_abs_tol", 1e-50, "Set the absolute tolerance.");
  params.addParam<int>("l_max_its", 10000, "Set the maximum number of iterations.");
  params.addParam<int>("print_level", 2, "Set the solver verbosity.");
  params.addParam<MFEMSolverName>("preconditioner", "Optional choice of preconditioner to use.");

  return params;
}

MFEMCGSolver::MFEMCGSolver(const InputParameters & parameters)
  : Moose::MFEM::LORLinearSolverBase<mfem::CGSolver>(parameters)
{
  ConstructSolver();
}

void
MFEMCGSolver::SetSolverParameters(mfem::CGSolver & solver)
{
  solver.iterative_mode = getParam<bool>("use_initial_guess");
  solver.SetRelTol(getParam<mfem::real_t>("l_tol"));
  solver.SetAbsTol(getParam<mfem::real_t>("l_abs_tol"));
  solver.SetMaxIter(getParam<int>("l_max_its"));
  solver.SetPrintLevel(getParam<int>("print_level"));
}

void
MFEMCGSolver::ConstructSolver()
{
  auto solver = std::make_unique<mfem::CGSolver>(getMFEMProblem().getComm());
  SetSolverParameters(*solver);
  SetPreconditioner(*solver);
  _solver = std::move(solver);
}

#endif

//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMGMRESSolver.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMGMRESSolver);

InputParameters
MFEMGMRESSolver::validParams()
{
  InputParameters params = Moose::MFEM::LinearSolverBase::validParams();
  params.addClassDescription("MFEM native solver for the iterative solution of MFEM equation "
                             "systems using the generalized minimal residual method.");
  params.set<bool>("use_initial_guess", /*quiet_mode=*/true) = true;
  params.addParam<mfem::real_t>("l_tol", 1e-5, "Set the relative tolerance.");
  params.addParam<mfem::real_t>("l_abs_tol", 1e-50, "Set the absolute tolerance.");
  params.addParam<int>("l_max_its", 10000, "Set the maximum number of iterations.");
  params.addParam<int>("print_level", 2, "Set the solver verbosity.");
  params.addParam<MFEMSolverName>("preconditioner", "Optional choice of preconditioner to use.");

  return params;
}

MFEMGMRESSolver::MFEMGMRESSolver(const InputParameters & parameters)
  : Moose::MFEM::LinearSolverBase(parameters), Moose::MFEM::LORInterface(parameters)
{
  ConstructSolver();
}

void
MFEMGMRESSolver::ConstructSolver()
{
  auto solver = std::make_unique<mfem::GMRESSolver>(getMFEMProblem().getComm());
  solver->iterative_mode = getParam<bool>("use_initial_guess");
  solver->SetRelTol(getParam<mfem::real_t>("l_tol"));
  solver->SetAbsTol(getParam<mfem::real_t>("l_abs_tol"));
  solver->SetMaxIter(getParam<int>("l_max_its"));
  solver->SetPrintLevel(getParam<int>("print_level"));
  SetPreconditioner(*solver);
  _solver = std::move(solver);
}

void
MFEMGMRESSolver::SetupLOR(Moose::MFEM::EquationSystem & equation_system)
{
  if (_lor && _preconditioner)
    mooseError("LOR solver cannot take a preconditioner");

  LORInterface::SetupLOR(equation_system);
  LORInterface * lor_preconditioner = GetPreconditionerLORInterface(*this);
  if (lor_preconditioner)
  {
    lor_preconditioner->SetupLOR(equation_system);
    SetPreconditioner(static_cast<mfem::GMRESSolver &>(*_solver));
  }
  else if (_lor)
  {
    auto lor_solver = new mfem::LORSolver<mfem::GMRESSolver>(*_a, _ess_tdofs);
    lor_solver->GetSolver().SetRelTol(getParam<mfem::real_t>("l_tol"));
    lor_solver->GetSolver().SetAbsTol(getParam<mfem::real_t>("l_abs_tol"));
    lor_solver->GetSolver().SetMaxIter(getParam<int>("l_max_its"));
    lor_solver->GetSolver().SetPrintLevel(getParam<int>("print_level"));

    _solver.reset(lor_solver);
  }
}

#endif

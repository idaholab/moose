//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "MFEMGMRESSolver.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMGMRESSolver);

InputParameters
MFEMGMRESSolver::validParams()
{
  InputParameters params = MFEMSolverBase::validParams();
  params.addClassDescription("MFEM native solver for the iterative solution of MFEM equation "
                             "systems using the generalized minimal residual method.");

  params.addParam<mfem::real_t>("l_tol", 1e-5, "Set the relative tolerance.");
  params.addParam<mfem::real_t>("l_abs_tol", 1e-50, "Set the absolute tolerance.");
  params.addParam<int>("l_max_its", 10000, "Set the maximum number of iterations.");
  params.addParam<int>("print_level", 2, "Set the solver verbosity.");
  params.addParam<UserObjectName>("preconditioner", "Optional choice of preconditioner to use.");

  return params;
}

MFEMGMRESSolver::MFEMGMRESSolver(const InputParameters & parameters) : MFEMSolverBase(parameters)
{
  constructSolver(parameters);
}

void
MFEMGMRESSolver::constructSolver(const InputParameters &)
{
  auto solver =
      std::make_unique<mfem::GMRESSolver>(getMFEMProblem().mesh().getMFEMParMesh().GetComm());
  solver->SetRelTol(getParam<mfem::real_t>("l_tol"));
  solver->SetAbsTol(getParam<mfem::real_t>("l_abs_tol"));
  solver->SetMaxIter(getParam<int>("l_max_its"));
  solver->SetPrintLevel(getParam<int>("print_level"));
  setPreconditioner(*solver);
  _solver = std::move(solver);
}

void
MFEMGMRESSolver::updateSolver(mfem::ParBilinearForm & a, mfem::Array<int> & tdofs)
{
  if (_lor && _preconditioner)
    mooseError("LOR solver cannot take a preconditioner");

  if (_preconditioner)
  {
    _preconditioner->updateSolver(a, tdofs);
    setPreconditioner(static_cast<mfem::GMRESSolver &>(*_solver));
  }
  else if (_lor)
  {
    auto lor_solver = new mfem::LORSolver<mfem::GMRESSolver>(a, tdofs);
    lor_solver->GetSolver().SetRelTol(getParam<mfem::real_t>("l_tol"));
    lor_solver->GetSolver().SetAbsTol(getParam<mfem::real_t>("l_abs_tol"));
    lor_solver->GetSolver().SetMaxIter(getParam<int>("l_max_its"));
    lor_solver->GetSolver().SetPrintLevel(getParam<int>("print_level"));

    _solver.reset(lor_solver);
  }
}

#endif

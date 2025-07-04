//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "MFEMHypreFGMRES.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMHypreFGMRES);

InputParameters
MFEMHypreFGMRES::validParams()
{
  InputParameters params = MFEMSolverBase::validParams();
  params.addClassDescription("Hypre solver for the iterative solution of MFEM equation systems "
                             "using the flexible generalized minimal residual method.");
  params.addParam<mfem::real_t>("l_tol", 1e-5, "Set the relative tolerance.");
  params.addParam<int>("l_max_its", 10000, "Set the maximum number of iterations.");
  params.addParam<int>("kdim", 10, "Set the k-dimension.");
  params.addParam<int>("print_level", 2, "Set the solver verbosity.");
  params.addParam<UserObjectName>("preconditioner", "Optional choice of preconditioner to use.");

  return params;
}

MFEMHypreFGMRES::MFEMHypreFGMRES(const InputParameters & parameters) : MFEMSolverBase(parameters)
{
  constructSolver(parameters);
}

void
MFEMHypreFGMRES::constructSolver(const InputParameters &)
{
  auto solver =
      std::make_unique<mfem::HypreFGMRES>(getMFEMProblem().mesh().getMFEMParMesh().GetComm());
  solver->SetTol(getParam<mfem::real_t>("l_tol"));
  solver->SetMaxIter(getParam<int>("l_max_its"));
  solver->SetKDim(getParam<int>("kdim"));
  solver->SetPrintLevel(getParam<int>("print_level"));
  setPreconditioner(*solver);
  _solver = std::move(solver);
}

void
MFEMHypreFGMRES::updateSolver(mfem::ParBilinearForm & a, mfem::Array<int> & tdofs)
{
  if (_lor && _preconditioner)
    mooseError("LOR solver cannot take a preconditioner");

  if (_preconditioner)
  {
    _preconditioner->updateSolver(a, tdofs);
    setPreconditioner(static_cast<mfem::HypreFGMRES &>(*_solver));
  }
  else if (_lor)
  {
    mfem::ParLORDiscretization lor_disc(a, tdofs);
    auto lor_solver = new mfem::LORSolver<mfem::HypreFGMRES>(
        lor_disc, getMFEMProblem().mesh().getMFEMParMesh().GetComm());
    lor_solver->GetSolver().SetTol(getParam<mfem::real_t>("l_tol"));
    lor_solver->GetSolver().SetMaxIter(getParam<int>("l_max_its"));
    lor_solver->GetSolver().SetKDim(getParam<int>("kdim"));
    lor_solver->GetSolver().SetPrintLevel(getParam<int>("print_level"));

    _solver.reset(lor_solver);
  }
}

#endif

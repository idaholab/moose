//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMHypreGMRES.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMHypreGMRES);

InputParameters
MFEMHypreGMRES::validParams()
{
  InputParameters params = Moose::MFEM::LinearSolverBase::validParams();
  params.addClassDescription("Hypre solver for the iterative solution of MFEM equation systems "
                             "using the generalized minimal residual method.");

  params.addParam<mfem::real_t>("l_tol", 1e-5, "Set the relative tolerance.");
  params.addParam<mfem::real_t>("l_abs_tol", 1e-50, "Set the absolute tolerance.");
  params.addParam<int>("l_max_its", 10000, "Set the maximum number of iterations.");
  params.addParam<int>("kdim", 10, "Set the k-dimension.");
  params.addParam<int>("print_level", 2, "Set the solver verbosity.");
  params.addParam<MFEMSolverName>("preconditioner", "Optional choice of preconditioner to use.");

  return params;
}

MFEMHypreGMRES::MFEMHypreGMRES(const InputParameters & parameters)
  : Moose::MFEM::LinearSolverBase(parameters), Moose::MFEM::LORInterface(parameters)
{
  ConstructSolver();
}

void
MFEMHypreGMRES::ConstructSolver()
{
  auto solver = std::make_unique<mfem::HypreGMRES>(getMFEMProblem().getComm());
  solver->iterative_mode = getParam<bool>("use_initial_guess");
  solver->SetTol(getParam<mfem::real_t>("l_tol"));
  solver->SetAbsTol(getParam<mfem::real_t>("l_abs_tol"));
  solver->SetMaxIter(getParam<int>("l_max_its"));
  solver->SetKDim(getParam<int>("kdim"));
  solver->SetPrintLevel(getParam<int>("print_level"));
  SetPreconditioner(*solver);
  _solver = std::move(solver);
}

void
MFEMHypreGMRES::SetupLOR()
{
  if (_lor && _preconditioner)
    mooseError("LOR solver cannot take a preconditioner");

  if (_equation_system->isComplex())
    mooseError("LOR solve is not supported for complex equation systems.");
  if (_equation_system->GetTestVarNames().size() > 1)
    mooseError("LOR solve is only supported for single-variable systems");

  const auto & test_var_name = _equation_system->GetTestVarNames().at(0);
  const auto & trial_var_name = _equation_system->GetTrialVarNames().at(0);
  mfem::ParBilinearForm & a = _equation_system->GetBilinearForm(test_var_name);
  mfem::ParGridFunction & trial_gf = *getMFEMProblem().getGridFunction(trial_var_name);

  mfem::Array<int> ess_bdr_markers(trial_gf.ParFESpace()->GetParMesh()->bdr_attributes.Max());
  ess_bdr_markers = 0;
  _equation_system->ApplyEssentialBC(trial_var_name, trial_gf, ess_bdr_markers);

  mfem::Array<int> ess_tdofs;
  a.ParFESpace()->GetEssentialTrueDofs(ess_bdr_markers, ess_tdofs);
  LORInterface * lor_preconditioner = GetPreconditionerLORInterface(*this);
  if (lor_preconditioner)
  {
    lor_preconditioner->SetupLOR();
    SetPreconditioner(static_cast<mfem::HypreGMRES &>(*_solver));
  }
  else if (_lor)
  {
    CheckSpectralEquivalence(a);
    mfem::ParLORDiscretization lor_disc(a, ess_tdofs);
    auto lor_solver = new mfem::LORSolver<mfem::HypreGMRES>(lor_disc, getMFEMProblem().getComm());
    lor_solver->GetSolver().SetTol(getParam<mfem::real_t>("l_tol"));
    lor_solver->GetSolver().SetAbsTol(getParam<mfem::real_t>("l_abs_tol"));
    lor_solver->GetSolver().SetMaxIter(getParam<int>("l_max_its"));
    lor_solver->GetSolver().SetKDim(getParam<int>("kdim"));
    lor_solver->GetSolver().SetPrintLevel(getParam<int>("print_level"));

    _solver.reset(lor_solver);
  }
}

#endif

//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "MFEMHypreBoomerAMG.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMHypreBoomerAMG);

InputParameters
MFEMHypreBoomerAMG::validParams()
{
  InputParameters params = MFEMSolverBase::validParams();
  params.addClassDescription("Hypre BoomerAMG solver and preconditioner for the iterative solution "
                             "of MFEM equation systems.");
  params.addParam<mfem::real_t>("l_tol", 1e-5, "Set the relative tolerance.");
  params.addParam<int>("l_max_its", 10000, "Set the maximum number of iterations.");
  params.addParam<int>("print_level", 2, "Set the solver verbosity.");
  params.addParam<UserObjectName>(
      "fespace", "H1 FESpace to use in HypreBoomerAMG setup for elasticity problems.");
  params.addParam<mfem::real_t>(
      "strength_threshold", 0.25, "HypreBoomerAMG strong threshold. Defaults to 0.25.");
  MooseEnum errmode("ignore=0 warn=1 abort=2", "abort", false);
  params.addParam<MooseEnum>("error_mode", errmode, "Set the behavior for treating hypre errors.");
  return params;
}

MFEMHypreBoomerAMG::MFEMHypreBoomerAMG(const InputParameters & parameters)
  : MFEMSolverBase(parameters),
    _mfem_fespace(isParamSetByUser("fespace") ? getUserObject<MFEMFESpace>("fespace").getFESpace()
                                              : nullptr)
{
  mfem::Hypre::Init();
  constructSolver(parameters);
}

void
MFEMHypreBoomerAMG::constructSolver(const InputParameters &)
{
  auto solver = std::make_unique<mfem::HypreBoomerAMG>();

  solver->SetTol(getParam<mfem::real_t>("l_tol"));
  solver->SetMaxIter(getParam<int>("l_max_its"));
  solver->SetPrintLevel(getParam<int>("print_level"));
  solver->SetStrengthThresh(getParam<mfem::real_t>("strength_threshold"));
  solver->SetErrorMode(mfem::HypreSolver::ErrorMode(int(getParam<MooseEnum>("error_mode"))));

  if (_mfem_fespace && !mfem::HypreUsingGPU())
    solver->SetElasticityOptions(_mfem_fespace.get());

  _solver = std::move(solver);
}

void
MFEMHypreBoomerAMG::updateSolver(mfem::ParBilinearForm & a, mfem::Array<int> & tdofs)
{
  if (_lor)
  {
    auto lor_solver = new mfem::LORSolver<mfem::HypreBoomerAMG>(a, tdofs);
    lor_solver->GetSolver().SetTol(getParam<mfem::real_t>("l_tol"));
    lor_solver->GetSolver().SetMaxIter(getParam<int>("l_max_its"));
    lor_solver->GetSolver().SetPrintLevel(getParam<int>("print_level"));
    lor_solver->GetSolver().SetStrengthThresh(getParam<mfem::real_t>("strength_threshold"));

    if (_mfem_fespace && !mfem::HypreUsingGPU())
      lor_solver->GetSolver().SetElasticityOptions(_mfem_fespace.get());

    _solver.reset(lor_solver);
  }
}

#endif

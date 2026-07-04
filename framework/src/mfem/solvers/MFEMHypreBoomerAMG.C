//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMHypreBoomerAMG.h"
#include "MFEMFESpace.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMHypreBoomerAMG);

InputParameters
MFEMHypreBoomerAMG::validParams()
{
  InputParameters params = Moose::MFEM::LinearSolverBase::validParams();
  params.addClassDescription("Hypre BoomerAMG solver and preconditioner for the iterative solution "
                             "of MFEM equation systems.");
  params.addParam<mfem::real_t>("l_tol", 1e-5, "Set the relative tolerance.");
  params.addParam<int>("l_max_its", 10000, "Set the maximum number of iterations.");
  params.addParam<int>("print_level", 2, "Set the solver verbosity.");
  params.addParam<MFEMFESpaceName>(
      "fespace", "H1 FESpace to use in HypreBoomerAMG setup for elasticity problems.");
  params.addParam<mfem::real_t>(
      "strength_threshold", 0.25, "HypreBoomerAMG strong threshold. Defaults to 0.25.");
  MooseEnum errmode("ignore=0 warn=1 abort=2", "abort", false);
  params.addParam<MooseEnum>("error_mode", errmode, "Set the behavior for treating hypre errors.");
  return params;
}

MFEMHypreBoomerAMG::MFEMHypreBoomerAMG(const InputParameters & parameters)
  : Moose::MFEM::LinearSolverBase(parameters),
    Moose::MFEM::LORInterface(parameters),
    _mfem_fespace(
        isParamSetByUser("fespace")
            ? getMFEMProblem()
                  .getMFEMObject<MFEMFESpace>("MFEMFESpace", getParam<MFEMFESpaceName>("fespace"))
                  .getFESpace()
            : nullptr)
{
  ConstructSolver();
}

MFEMHypreBoomerAMG::~MFEMHypreBoomerAMG() { _solver.reset(); }

void
MFEMHypreBoomerAMG::ConstructSolver()
{
  auto solver = std::make_unique<mfem::HypreBoomerAMG>();

  solver->iterative_mode = getParam<bool>("use_initial_guess");
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
MFEMHypreBoomerAMG::SetupLOR(Moose::MFEM::EquationSystem & equation_system)
{
  if (_lor)
  {
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

    CheckSpectralEquivalence(a);
    mfem::Array<int> ess_tdofs;
    a.ParFESpace()->GetEssentialTrueDofs(ess_bdr_markers, ess_tdofs);
    auto lor_solver = new mfem::LORSolver<mfem::HypreBoomerAMG>(a, ess_tdofs);
    lor_solver->GetSolver().SetTol(getParam<mfem::real_t>("l_tol"));
    lor_solver->GetSolver().SetMaxIter(getParam<int>("l_max_its"));
    lor_solver->GetSolver().SetPrintLevel(getParam<int>("print_level"));
    lor_solver->GetSolver().SetStrengthThresh(getParam<mfem::real_t>("strength_threshold"));

    /// HypreBoomerAMG options for elasticity problems are not compatible with GPU execution
    if (_mfem_fespace && !mfem::HypreUsingGPU())
      lor_solver->GetSolver().SetElasticityOptions(_mfem_fespace.get());

    _solver.reset(lor_solver);
  }
}

#endif

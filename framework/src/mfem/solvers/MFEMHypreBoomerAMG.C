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
MFEMHypreBoomerAMG::SetSolverParameters(mfem::Solver & solver)
{
  auto & mfem_solver = static_cast<mfem::HypreBoomerAMG &>(solver);
  mfem_solver.iterative_mode = getParam<bool>("use_initial_guess");
  mfem_solver.SetTol(getParam<mfem::real_t>("l_tol"));
  mfem_solver.SetMaxIter(getParam<int>("l_max_its"));
  mfem_solver.SetPrintLevel(getParam<int>("print_level"));
  mfem_solver.SetStrengthThresh(getParam<mfem::real_t>("strength_threshold"));
  mfem_solver.SetErrorMode(mfem::HypreSolver::ErrorMode(int(getParam<MooseEnum>("error_mode"))));

  if (_mfem_fespace && !mfem::HypreUsingGPU())
    mfem_solver.SetElasticityOptions(_mfem_fespace.get());
}

void
MFEMHypreBoomerAMG::ConstructSolver()
{
  auto solver = std::make_unique<mfem::HypreBoomerAMG>();
  SetSolverParameters(*solver);
  _solver = std::move(solver);
}

void
MFEMHypreBoomerAMG::Update()
{
  if (IsLOR(*this))
    LORInterface::SetupLOR<mfem::HypreBoomerAMG>(*this, *_equation_system);
}

#endif

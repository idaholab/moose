#pragma once
#include "MFEMHypreBoomerAMG.h"
#include "MFEMProblem.h"

registerMooseObject("PlatypusApp", MFEMHypreBoomerAMG);

InputParameters
MFEMHypreBoomerAMG::validParams()
{
  InputParameters params = MFEMSolverBase::validParams();
  params.addParam<double>("l_tol", 1e-5, "Set the relative tolerance.");
  params.addParam<int>("l_max_its", 10000, "Set the maximum number of iterations.");
  params.addParam<int>("print_level", 2, "Set the solver verbosity.");
  return params;
}

MFEMHypreBoomerAMG::MFEMHypreBoomerAMG(const InputParameters & parameters)
  : MFEMSolverBase(parameters)
{
  constructSolver(parameters);
}

void
MFEMHypreBoomerAMG::constructSolver(const InputParameters & parameters)
{
  _solver = std::make_shared<mfem::HypreBoomerAMG>();

  _solver->SetTol(getParam<double>("l_tol"));
  _solver->SetMaxIter(getParam<int>("l_max_its"));
  _solver->SetPrintLevel(getParam<int>("print_level"));
}
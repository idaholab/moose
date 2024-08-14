#pragma once
#include "MFEMHypreBoomerAMG.h"
#include "MFEMProblem.h"

registerMooseObject("PlatypusApp", MFEMHypreBoomerAMG);

InputParameters
MFEMHypreBoomerAMG::validParams()
{
  InputParameters params = MFEMSolverBase::validParams();
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
  _preconditioner = std::make_shared<mfem::HypreBoomerAMG>();
  _preconditioner->SetPrintLevel(getParam<int>("print_level"));
}
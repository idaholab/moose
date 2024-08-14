#pragma once
#include "MFEMHypreBoomerAMG.h"
#include "MFEMProblem.h"

registerMooseObject("PlatypusApp", MFEMHypreBoomerAMG);

InputParameters
MFEMHypreBoomerAMG::validParams()
{
  InputParameters params = MFEMPreconditionerBase::validParams();
  params.addParam<int>("print_level", 2, "Set the solver verbosity.");
  return params;
}

MFEMHypreBoomerAMG::MFEMHypreBoomerAMG(const InputParameters & parameters)
  : MFEMPreconditionerBase(parameters)
{
  constructPreconditioner(parameters);
}

void
MFEMHypreBoomerAMG::constructPreconditioner(const InputParameters & parameters)
{
  _preconditioner = std::make_shared<mfem::HypreBoomerAMG>();
  _preconditioner->SetPrintLevel(getParam<int>("print_level"));
}
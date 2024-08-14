#pragma once
#include "MFEMHypreBoomerAMG.h"
#include "MFEMProblem.h"

registerMooseObject("PlatypusApp", MFEMHypreBoomerAMG);

InputParameters
MFEMHypreBoomerAMG::validParams()
{
  InputParameters params = MFEMPreconditionerBase::validParams();
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
  _preconditioner->SetPrintLevel(2);
}
#pragma once
#include "MFEMHypreAMS.h"

registerMooseObject("PlatypusApp", MFEMHypreAMS);

InputParameters
MFEMHypreAMS::validParams()
{
  InputParameters params = MFEMPreconditionerBase::validParams();
  params.addParam<UserObjectName>("fespace", "H(curl) FESpace to use in HypreAMS setup.");
  return params;
}

MFEMHypreAMS::MFEMHypreAMS(const InputParameters & parameters)
  : MFEMPreconditionerBase(parameters), _mfem_fespace(getUserObject<MFEMFESpace>("fespace"))
{
  constructPreconditioner(parameters);
}

void
MFEMHypreAMS::constructPreconditioner(const InputParameters & parameters)
{
  _preconditioner = std::make_shared<mfem::HypreAMS>(_mfem_fespace.getFESpace().get());
  _preconditioner->SetPrintLevel(2);
}
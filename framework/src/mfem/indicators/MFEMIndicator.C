//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "MFEMIndicator.h"

registerMooseObject("MooseApp", MFEMIndicator);

// static method
InputParameters
MFEMIndicator::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params.registerBase("Indicator");

  params.addRequiredParam<std::string>("variable", "Variable to perform amr with");
  params.addRequiredParam<std::string>("kernel", "Kernel to perform amr with");

  return params;
}

MFEMIndicator::MFEMIndicator(const InputParameters & params)
  : MFEMGeneralUserObject(params),
    _variable_name(getParam<std::string>("variable")),
    _variable(getUserObject<MFEMVariable>("variable")),
    _kernel_name(getParam<std::string>("kernel"))
{
}

std::shared_ptr<mfem::ParFiniteElementSpace>
MFEMIndicator::getFESpace() const
{
  // MFEMVariable::getFESpace() returns a reference to the MFEMFESpace
  // and we piggyback from this to get the underlying shared ptr
  return _variable.getFESpace().getFESpace();
}

#endif

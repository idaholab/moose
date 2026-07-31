//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMWeakForm.h"

registerMooseObject("MooseApp", MFEMWeakForm);

InputParameters
MFEMWeakForm::validParams()
{
  InputParameters params = MFEMObject::validParams();
  params.registerBase("MFEMWeakForm");
  params.registerSystemAttributeName("MFEMWeakForm");
  return params;
}

MFEMWeakForm::MFEMWeakForm(const InputParameters & parameters) : MFEMObject(parameters) {}

std::shared_ptr<Moose::MFEM::EquationSystem>
MFEMWeakForm::createEquationSystem() const
{
  _equation_system = std::make_shared<Moose::MFEM::EquationSystem>();
  return _equation_system;
}

#endif

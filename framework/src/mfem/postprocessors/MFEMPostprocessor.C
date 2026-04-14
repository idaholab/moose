//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMPostprocessor.h"

InputParameters
MFEMPostprocessor::validParams()
{
  InputParameters params = MFEMExecutedObject::validParams();
  params += Postprocessor::validParams();
  params.registerSystemAttributeName("MFEMExecutedObject");
  return params;
}

MFEMPostprocessor::MFEMPostprocessor(const InputParameters & parameters)
  : MFEMExecutedObject(parameters), Postprocessor(this)
{
}

std::optional<std::string>
MFEMPostprocessor::suppliedPostprocessorName() const
{
  return name();
}

#endif

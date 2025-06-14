//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "MFEMPostprocessor.h"

InputParameters
MFEMPostprocessor::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params += Postprocessor::validParams();
  return params;
}

MFEMPostprocessor::MFEMPostprocessor(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters), Postprocessor(this)
{
}

#endif

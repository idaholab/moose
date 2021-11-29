//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneralPostprocessor.h"

InputParameters
GeneralPostprocessor::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params += Postprocessor::validParams();
  return params;
}

GeneralPostprocessor::GeneralPostprocessor(const InputParameters & parameters)
  : GeneralUserObject(parameters), Postprocessor(this)
{
}

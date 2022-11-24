//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolidProperties.h"

InputParameters
SolidProperties::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addParam<bool>(
      "allow_imperfect_jacobians",
      false,
      "true to allow unimplemented property derivative terms to be set to zero for the AD API");
  params.registerBase("SolidProperties");
  return params;
}

SolidProperties::SolidProperties(const InputParameters & parameters)
  : ThreadedGeneralUserObject(parameters),
    _allow_imperfect_jacobians(getParam<bool>("allow_imperfect_jacobians"))
{
}

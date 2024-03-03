//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HEVPStrengthUOBase.h"

InputParameters
HEVPStrengthUOBase::validParams()
{
  InputParameters params = DiscreteElementUserObject::validParams();
  params.addParam<std::string>("intvar_prop_name",
                               "Name of internal variable property to "
                               "calculate material resistance: Same as "
                               "internal variable user object");
  params.addClassDescription("User Object to compute material resistance");
  return params;
}

HEVPStrengthUOBase::HEVPStrengthUOBase(const InputParameters & parameters)
  : DiscreteElementUserObject(parameters),
    _intvar_prop_name(getParam<std::string>("intvar_prop_name")),
    _intvar(getMaterialPropertyByName<Real>(_intvar_prop_name))
{
}

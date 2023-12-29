//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HEVPInternalVarUOBase.h"

InputParameters
HEVPInternalVarUOBase::validParams()
{
  InputParameters params = DiscreteElementUserObject::validParams();
  params.addParam<std::string>(
      "intvar_rate_prop_name",
      "Name of internal variable property: Same as internal variable rate user object");
  params.addClassDescription("User Object to integrate internal variable");
  return params;
}

HEVPInternalVarUOBase::HEVPInternalVarUOBase(const InputParameters & parameters)
  : DiscreteElementUserObject(parameters),
    _intvar_rate_prop_name(getParam<std::string>("intvar_rate_prop_name")),
    _intvar_rate(getMaterialPropertyByName<Real>(_intvar_rate_prop_name)),
    _this_old(getMaterialPropertyOldByName<Real>(_name))
{
}

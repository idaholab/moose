/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "HEVPInternalVarUOBase.h"

template <>
InputParameters
validParams<HEVPInternalVarUOBase>()
{
  InputParameters params = validParams<DiscreteElementUserObject>();
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

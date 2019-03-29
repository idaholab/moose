//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceValueUserObjectAux.h"

registerMooseObject("MooseApp", InterfaceValueUserObjectAux);

template <>
InputParameters
validParams<InterfaceValueUserObjectAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<UserObjectName>("interface_uo_name",
                                          "The name of the interface user object to use");
  params.addClassDescription("Get stored value from the specified InterfaceQpValueUserObject.");

  return params;
}

InterfaceValueUserObjectAux::InterfaceValueUserObjectAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _interface_uo(getUserObject<InterfaceQpValueUserObject>("interface_uo_name"))
{
}

Real
InterfaceValueUserObjectAux::computeValue()
{
  return _interface_uo.getQpValue(_current_elem->id(), _current_side, _qp);
}

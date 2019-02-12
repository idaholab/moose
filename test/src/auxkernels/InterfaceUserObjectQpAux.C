//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceUserObjectQpAux.h"

registerMooseObject("MooseTestApp", InterfaceUserObjectQpAux);

template <>
InputParameters
validParams<InterfaceUserObjectQpAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<UserObjectName>("interface_uo_name",
                                          "the name of the interface user object to use");

  return params;
}

InterfaceUserObjectQpAux::InterfaceUserObjectQpAux(const InputParameters & parameters)
  : AuxKernel(parameters), _interface_uo(getUserObject<InterfaceUO_QP>("interface_uo_name"))
{
}

Real
InterfaceUserObjectQpAux::computeValue()
{
  return _interface_uo.getQpValue(_current_elem->id(), _current_side, _qp);
}

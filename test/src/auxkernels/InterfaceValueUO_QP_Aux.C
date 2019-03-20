//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceValueUO_QP_Aux.h"

registerMooseObject("MooseTestApp", InterfaceValueUO_QP_Aux);

template <>
InputParameters
validParams<InterfaceValueUO_QP_Aux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<UserObjectName>("interface_uo_name",
                                          "The name of the interface user object to use");

  return params;
}

InterfaceValueUO_QP_Aux::InterfaceValueUO_QP_Aux(const InputParameters & parameters)
  : AuxKernel(parameters), _interface_uo(getUserObject<InterfaceValueUO_QP>("interface_uo_name"))
{
}

Real
InterfaceValueUO_QP_Aux::computeValue()
{
  return _interface_uo.getQpValue(_current_elem->id(), _current_side, _qp);
}

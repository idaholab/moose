//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "JumpInterfaceMaterial.h"

registerMooseObject("MooseTestApp", JumpInterfaceMaterial);

template <>
InputParameters
validParams<JumpInterfaceMaterial>()
{
  InputParameters params = validParams<InterfaceMaterial>();
  params.addClassDescription("Calculates a variable's jump value across an interface.");
  params.addRequiredCoupledVar("var", "Name of the variable");
  params.addRequiredCoupledVar("neighbor_var", "Name of the neighbor variable");
  return params;
}

JumpInterfaceMaterial::JumpInterfaceMaterial(const InputParameters & parameters)
  : InterfaceMaterial(parameters),
    _value(coupledValue("var")),
    _neighbor_value(coupledNeighborValue("neighbor_var")),
    _jump(declareProperty<Real>("jump"))
{
}

void
JumpInterfaceMaterial::computeQpProperties()
{
  _jump[_qp] = _value[_qp] - _neighbor_value[_qp];
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PresetBC.h"

template <>
InputParameters
validParams<PresetBC>()
{
  InputParameters p = validParams<NodalBC>();
  p.addRequiredParam<Real>("value", "Value of the BC");
  p.declareControllable("value");
  p.addClassDescription(
      "Similar to DirichletBC except the value is applied before the solve begins");
  return p;
}

PresetBC::PresetBC(const InputParameters & parameters)
  : PresetNodalBC(parameters), _value(getParam<Real>("value"))
{
}

Real
PresetBC::computeQpValue()
{
  return _value;
}

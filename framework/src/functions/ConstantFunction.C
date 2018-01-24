//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConstantFunction.h"

template <>
InputParameters
validParams<ConstantFunction>()
{
  InputParameters params = validParams<Function>();
  params.addParam<Real>("value", 0.0, "The constant value");
  params.declareControllable("value");
  return params;
}

ConstantFunction::ConstantFunction(const InputParameters & parameters)
  : Function(parameters), _value(getParam<Real>("value"))
{
}

Real
ConstantFunction::value(Real, const Point &)
{
  return _value;
}

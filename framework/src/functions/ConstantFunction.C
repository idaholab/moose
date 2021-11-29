//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConstantFunction.h"

registerMooseObject("MooseApp", ConstantFunction);

InputParameters
ConstantFunction::validParams()
{
  InputParameters params = Function::validParams();
  params.addClassDescription(
      "A function that returns a constant value as defined by an input parameter.");
  params.addParam<Real>("value", 0.0, "The constant value");
  params.declareControllable("value");
  return params;
}

ConstantFunction::ConstantFunction(const InputParameters & parameters)
  : Function(parameters), _value(getParam<Real>("value"))
{
}

Real
ConstantFunction::value(Real, const Point &) const
{
  return _value;
}

ADReal
ConstantFunction::value(const ADReal &, const ADPoint &) const
{
  return _value;
}

Real
ConstantFunction::timeDerivative(Real /*t*/, const Point & /*p*/) const
{
  return 0;
}

RealVectorValue
ConstantFunction::gradient(Real /*t*/, const Point & /*p*/) const
{
  return RealVectorValue(0);
}

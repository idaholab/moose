//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionIC.h"
#include "Function.h"

registerMooseObject("MooseApp", FunctionIC);

InputParameters
FunctionIC::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addRequiredParam<FunctionName>("function", "The initial condition function.");

  params.addClassDescription("An initial condition that uses a normal function of x, y, z to "
                             "produce values (and optionally gradients) for a field variable.");
  params.addParam<Real>("scaling_factor", 1, "Scaling factor to apply on the function");

  return params;
}

FunctionIC::FunctionIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _func(getFunction("function")),
    _scaling(getParam<Real>("scaling_factor"))
{
}

Real
FunctionIC::value(const Point & p)
{
  return _scaling * _func.value(_t, p);
}

RealGradient
FunctionIC::gradient(const Point & p)
{
  return _scaling * _func.gradient(_t, p);
}

const FunctionName
FunctionIC::functionName() const
{
  return _func.name();
}

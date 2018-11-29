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

template <>
InputParameters
validParams<FunctionIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addRequiredParam<FunctionName>("function", "The initial condition function.");
  params.addParam<FunctionName>("function_dot", "The initial time derivative function.");

  params.addClassDescription("An initial condition that uses a normal function of x, y, z to "
                             "produce values (and optionally gradients) for a field variable.");
  return params;
}

FunctionIC::FunctionIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _func(getFunction("function")),
    _func_dot(isParamValid("function_dot") ? &getFunction("function_dot") : NULL)
{
}

Real
FunctionIC::value(const Point & p)
{
  return _func.value(_t, p);
}

RealGradient
FunctionIC::gradient(const Point & p)
{
  return _func.gradient(_t, p);
}

Real
FunctionIC::valueDot(const Point & p)
{
  if (!_func_dot)
    return 0.0;

  return _func_dot->value(_t, p);
}

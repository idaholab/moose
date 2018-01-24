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

template <>
InputParameters
validParams<FunctionIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addRequiredParam<FunctionName>("function", "The initial condition function.");
  return params;
}

FunctionIC::FunctionIC(const InputParameters & parameters)
  : InitialCondition(parameters), _func(getFunction("function"))
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

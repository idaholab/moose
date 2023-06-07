//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VariableFunctionProductIC.h"
#include "Function.h"

registerMooseObject("ThermalHydraulicsApp", VariableFunctionProductIC);

InputParameters
VariableFunctionProductIC::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addRequiredCoupledVar("var", "Coupled variable");
  params.addRequiredParam<FunctionName>("fn", "User function");
  params.addClassDescription(
      "Sets the initial condition as the product of a variable and a function");
  return params;
}

VariableFunctionProductIC::VariableFunctionProductIC(const InputParameters & parameters)
  : InitialCondition(parameters), _var(coupledValue("var")), _fn(getFunction("fn"))
{
}

Real
VariableFunctionProductIC::value(const Point & p)
{
  return _var[_qp] * _fn.value(_t, p);
}

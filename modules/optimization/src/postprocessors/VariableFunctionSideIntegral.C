//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VariableFunctionSideIntegral.h"

registerMooseObject("isopodApp", VariableFunctionSideIntegral);

InputParameters
VariableFunctionSideIntegral::validParams()
{
  InputParameters params = FunctionSideIntegral::validParams();
  params.addClassDescription(
      "Computes the inner product of a function and variable over a boundary.");

  params.addRequiredCoupledVar("variable",
                               "The name of the variable being multiplied by the function");
  return params;
}

VariableFunctionSideIntegral::VariableFunctionSideIntegral(const InputParameters & parameters)
  : FunctionSideIntegral(parameters), _u(coupledValue("variable"))
{
}

Real
VariableFunctionSideIntegral::computeQpIntegral()
{
  return FunctionSideIntegral::computeQpIntegral() * _u[_qp];
}

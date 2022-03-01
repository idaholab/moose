//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionSideIntegral.h"
#include "Function.h"

registerMooseObject("MooseApp", FunctionSideIntegral);

InputParameters
FunctionSideIntegral::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();
  params.addClassDescription("Computes the integral of a function over a boundary.");
  params.addParam<FunctionName>(
      "function",
      1.0,
      "This postprocessor will return the integral of this function over the boundary");
  return params;
}

FunctionSideIntegral::FunctionSideIntegral(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters), _func(getFunction("function"))
{
}

void
FunctionSideIntegral::threadJoin(const UserObject & y)
{
  const FunctionSideIntegral & pps = static_cast<const FunctionSideIntegral &>(y);
  _integral_value += pps._integral_value;
}

Real
FunctionSideIntegral::computeQpIntegral()
{
  return _func.value(_t, _q_point[_qp]);
}

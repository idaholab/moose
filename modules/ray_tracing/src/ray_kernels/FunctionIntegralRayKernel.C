//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionIntegralRayKernel.h"

#include "Function.h"

registerMooseObject("RayTracingApp", FunctionIntegralRayKernel);

InputParameters
FunctionIntegralRayKernel::validParams()
{
  InputParameters params = IntegralRayKernel::validParams();

  params.addClassDescription("Integrates a function along a Ray.");

  params.addRequiredParam<FunctionName>("function", "The function");

  return params;
}

FunctionIntegralRayKernel::FunctionIntegralRayKernel(const InputParameters & params)
  : IntegralRayKernel(params), _function(getFunction("function"))
{
}

Real
FunctionIntegralRayKernel::computeQpIntegral()
{
  return _function.value(_t, _q_point[_qp]);
}

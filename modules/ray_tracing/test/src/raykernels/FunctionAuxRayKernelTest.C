//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionAuxRayKernelTest.h"

#include "Function.h"

registerMooseObject("RayTracingTestApp", FunctionAuxRayKernelTest);

InputParameters
FunctionAuxRayKernelTest::validParams()
{
  auto params = AuxRayKernel::validParams();
  params.addRequiredParam<FunctionName>("function", "The function to use as the value");
  return params;
}

FunctionAuxRayKernelTest::FunctionAuxRayKernelTest(const InputParameters & params)
  : AuxRayKernel(params), _func(getFunction("function"))
{
}

void
FunctionAuxRayKernelTest::onSegment()
{
  const Point midpoint = 0.5 * (_current_segment_start + _current_segment_end);
  addValue(_func.value(_t, midpoint));
}

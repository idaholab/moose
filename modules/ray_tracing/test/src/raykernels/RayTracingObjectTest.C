//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RayTracingObjectTest.h"

#include "RepeatableRayStudy.h"

registerMooseObject("RayTracingTestApp", RayTracingObjectTest);

InputParameters
RayTracingObjectTest::validParams()
{
  auto params = NullRayKernel::validParams();

  params.addParam<bool>("get_study_bad", false, "Tests a bad getStudy() cast");

  return params;
}

RayTracingObjectTest::RayTracingObjectTest(const InputParameters & params) : NullRayKernel(params)
{
  if (getParam<bool>("get_study_bad"))
    getStudy<RepeatableRayStudy>();
}

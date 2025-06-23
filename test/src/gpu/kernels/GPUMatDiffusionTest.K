//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUMatDiffusionTest.h"

registerMooseObject("MooseTestApp", GPUMatDiffusionTest);

InputParameters
GPUMatDiffusionTest::validParams()
{
  InputParameters params = GPUKernel::validParams();
  params.addRequiredParam<MaterialPropertyName>(
      "prop_name", "the name of the material property we are going to use");

  MooseEnum prop_state("current old older", "current");
  params.addParam<MooseEnum>(
      "prop_state", prop_state, "Declares which property state we should retrieve");
  return params;
}

GPUMatDiffusionTest::GPUMatDiffusionTest(const InputParameters & parameters) : GPUKernel(parameters)
{
  MooseEnum prop_state = getParam<MooseEnum>("prop_state");

  if (prop_state == "current")
    _diff = getGPUMaterialProperty<Real>("prop_name");
  else if (prop_state == "old")
    _diff = getGPUMaterialPropertyOld<Real>("prop_name");
  else if (prop_state == "older")
    _diff = getGPUMaterialPropertyOlder<Real>("prop_name");
}

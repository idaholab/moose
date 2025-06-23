//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUStatefulSpatialTest.h"

registerMooseObject("MooseTestApp", GPUStatefulSpatialTest);

InputParameters
GPUStatefulSpatialTest::validParams()
{
  InputParameters params = GPUMaterial::validParams();
  params.addParam<MaterialPropertyName>("prop_name", "thermal_conductivity", "Property name");
  params.addParam<bool>("declare_older", false, "Whether older property will be declared");
  return params;
}

GPUStatefulSpatialTest::GPUStatefulSpatialTest(const InputParameters & parameters)
  : GPUMaterial(parameters),
    _thermal_conductivity(declareGPUProperty<Real>(getParam<MaterialPropertyName>("prop_name"))),
    _thermal_conductivity_old(
        getGPUMaterialPropertyOld<Real>(getParam<MaterialPropertyName>("prop_name")))
{
  if (getParam<bool>("declare_older"))
    getGPUMaterialPropertyOlder<Real>(getParam<MaterialPropertyName>("prop_name"));
}

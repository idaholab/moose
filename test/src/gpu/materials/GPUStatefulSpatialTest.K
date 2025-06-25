//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUStatefulSpatialTest.h"

registerMooseObject("MooseTestApp", KokkosStatefulSpatialTest);

InputParameters
KokkosStatefulSpatialTest::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<MaterialPropertyName>("prop_name", "thermal_conductivity", "Property name");
  params.addParam<bool>("declare_older", false, "Whether older property will be declared");
  return params;
}

KokkosStatefulSpatialTest::KokkosStatefulSpatialTest(const InputParameters & parameters)
  : Material(parameters),
    _thermal_conductivity(declareKokkosProperty<Real>(getParam<MaterialPropertyName>("prop_name"))),
    _thermal_conductivity_old(
        getKokkosMaterialPropertyOld<Real>(getParam<MaterialPropertyName>("prop_name")))
{
  if (getParam<bool>("declare_older"))
    getKokkosMaterialPropertyOlder<Real>(getParam<MaterialPropertyName>("prop_name"));
}

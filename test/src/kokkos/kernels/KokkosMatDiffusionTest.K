//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosMatDiffusionTest.h"

registerMooseObject("MooseTestApp", KokkosMatDiffusionTest);

InputParameters
KokkosMatDiffusionTest::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addRequiredParam<MaterialPropertyName>(
      "prop_name", "the name of the material property we are going to use");

  MooseEnum prop_state("current old older", "current");
  params.addParam<MooseEnum>(
      "prop_state", prop_state, "Declares which property state we should retrieve");
  return params;
}

KokkosMatDiffusionTest::KokkosMatDiffusionTest(const InputParameters & parameters)
  : Kernel(parameters)
{
  MooseEnum prop_state = getParam<MooseEnum>("prop_state");

  if (prop_state == "current")
    _diff = getKokkosMaterialProperty<Real>("prop_name");
  else if (prop_state == "old")
    _diff = getKokkosMaterialPropertyOld<Real>("prop_name");
  else if (prop_state == "older")
    _diff = getKokkosMaterialPropertyOlder<Real>("prop_name");
}

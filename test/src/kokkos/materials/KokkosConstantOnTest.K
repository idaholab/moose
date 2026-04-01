//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosConstantOnTest.h"

registerKokkosMaterial("MooseTestApp", KokkosConstantOnTest);

InputParameters
KokkosConstantOnTest::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredParam<std::string>("prop_name",
                                       "The name of the property this material will have");
  return params;
}

KokkosConstantOnTest::KokkosConstantOnTest(const InputParameters & parameters)
  : Material(parameters)
{
  _property = declareKokkosProperty<Real>(getParam<std::string>("prop_name"));
}

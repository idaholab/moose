//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosLazyTest.h"

registerKokkosMaterial("MooseTestApp", KokkosLazyTest);

InputParameters
KokkosLazyTest::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<std::vector<std::string>>("prop_names",
                                            "The names of the properties this material will have");
  return params;
}

KokkosLazyTest::KokkosLazyTest(const InputParameters & parameters)
  : Material(parameters), _prop_names(getParam<std::vector<std::string>>("prop_names"))
{
  unsigned int num_names = _prop_names.size();

  _num_props = num_names;

  _properties.create(num_names);

  for (unsigned int i = 0; i < _num_props; ++i)
    _properties[i] = declareKokkosLazyProperty<Real>(_prop_names[i]);
}

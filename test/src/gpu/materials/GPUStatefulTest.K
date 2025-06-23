//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUStatefulTest.h"

registerMooseObject("MooseTestApp", GPUStatefulTest);

InputParameters
GPUStatefulTest::validParams()
{
  InputParameters params = GPUMaterial::validParams();
  params.addParam<std::vector<std::string>>("prop_names",
                                            "The names of the properties this material will have");
  params.addParam<std::vector<Real>>("prop_values",
                                     "The values associated with the named properties");
  params.addCoupledVar("coupled", "Coupled Value to be used in initQpStatefulProperties()");
  return params;
}

GPUStatefulTest::GPUStatefulTest(const InputParameters & parameters)
  : GPUMaterial(parameters),
    _coupled_val(isParamValid("coupled") ? GPUCoupledValue("coupled") : GPUZeroValue()),
    _prop_names(getParam<std::vector<std::string>>("prop_names")),
    _prop_values(getParam<std::vector<Real>>("prop_values"))
{
  unsigned int num_names = _prop_names.size();
  unsigned int num_values = _prop_values.size();

  if (num_names != num_values)
    mooseError(
        "Number of prop_names must match the number of prop_values for StatefulTest material!");

  _num_props = num_names;

  _properties.create(num_names);
  _properties_old.create(num_names);
  _properties_older.create(num_names);

  for (unsigned int i = 0; i < _num_props; ++i)
  {
    _properties[i] = declareGPUProperty<Real>(_prop_names[i]);
    _properties_old[i] = getGPUMaterialPropertyOld<Real>(_prop_names[i]);
    _properties_older[i] = getGPUMaterialPropertyOlder<Real>(_prop_names[i]);
  }
}

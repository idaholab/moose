//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosGenericConstantMaterial.h"

registerKokkosMaterial("MooseApp", KokkosGenericConstantMaterial);

InputParameters
KokkosGenericConstantMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "Declares material properties based on names and values prescribed by input parameters.");
  params.addRequiredParam<std::vector<std::string>>(
      "prop_names", "The names of the properties this material will have");
  params.addRequiredParam<std::vector<Real>>("prop_values",
                                             "The values associated with the named properties");
  params.declareControllable("prop_values");

  return params;
}

KokkosGenericConstantMaterial::KokkosGenericConstantMaterial(const InputParameters & parameters)
  : Material(parameters),
    _prop_names(getParam<std::vector<std::string>>("prop_names")),
    _prop_values(getParam<std::vector<Real>>("prop_values")),
    _num_props(_prop_names.size())
{
  unsigned int num_names = _prop_names.size();
  unsigned int num_values = _prop_values.size();

  if (num_names != num_values)
    paramError("prop_names", "Size must match the number of prop_values.");

  _props.create(_num_props);

  for (unsigned int i = 0; i < _num_props; ++i)
    _props[i] = declareKokkosProperty<Real>(_prop_names[i]);
}

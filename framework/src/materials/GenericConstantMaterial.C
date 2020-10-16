//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GenericConstantMaterial.h"

registerMooseObject("MooseApp", GenericConstantMaterial);
registerMooseObject("MooseApp", ADGenericConstantMaterial);

template <bool is_ad>
InputParameters
GenericConstantMaterialTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "Declares material properties based on names and values prescribed by input parameters.");
  params.addRequiredParam<std::vector<std::string>>(
      "prop_names", "The names of the properties this material will have");
  params.addRequiredParam<std::vector<Real>>("prop_values",
                                             "The values associated with the named properties");
  params.set<MooseEnum>("constant_on") = "SUBDOMAIN";
  params.declareControllable("prop_values");
  return params;
}

template <bool is_ad>
GenericConstantMaterialTempl<is_ad>::GenericConstantMaterialTempl(
    const InputParameters & parameters)
  : Material(parameters),
    _prop_names(getParam<std::vector<std::string>>("prop_names")),
    _prop_values(getParam<std::vector<Real>>("prop_values"))
{
  unsigned int num_names = _prop_names.size();
  unsigned int num_values = _prop_values.size();

  if (num_names != num_values)
    mooseError(
        "Number of prop_names must match the number of prop_values for a GenericConstantMaterial!");

  _num_props = num_names;

  _properties.resize(num_names);

  for (unsigned int i = 0; i < _num_props; i++)
    _properties[i] = &declareGenericProperty<Real, is_ad>(_prop_names[i]);
}

template <bool is_ad>
void
GenericConstantMaterialTempl<is_ad>::initQpStatefulProperties()
{
  computeQpProperties();
}

template <bool is_ad>
void
GenericConstantMaterialTempl<is_ad>::computeQpProperties()
{
  for (unsigned int i = 0; i < _num_props; i++)
    (*_properties[i])[_qp] = _prop_values[i];
}

template class GenericConstantMaterialTempl<false>;
template class GenericConstantMaterialTempl<true>;

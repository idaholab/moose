//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GenericConstantStdVectorMaterial.h"

registerMooseObject("MooseApp", GenericConstantStdVectorMaterial);
registerMooseObject("MooseApp", ADGenericConstantStdVectorMaterial);

template <bool is_ad>
InputParameters
GenericConstantStdVectorMaterialTempl<is_ad>::validParams()
{

  InputParameters params = Material::validParams();
  params.addClassDescription("Declares material properties based on names and vector values "
                             "prescribed by input parameters.");
  params.addRequiredParam<std::vector<std::string>>(
      "prop_names", "The names of the properties this material will have");
  params.addRequiredParam<std::vector<std::vector<Real>>>(
      "prop_values", "The values associated with the named properties. ");
  params.set<MooseEnum>("constant_on") = "SUBDOMAIN";
  return params;
}

template <bool is_ad>
GenericConstantStdVectorMaterialTempl<is_ad>::GenericConstantStdVectorMaterialTempl(
    const InputParameters & parameters)
  : Material(parameters),
    VariableSizeMaterialPropertiesInterface(parameters),
    _prop_names(getParam<std::vector<std::string>>("prop_names")),
    _prop_values(getParam<std::vector<std::vector<Real>>>("prop_values"))
{
  const auto num_names = _prop_names.size();
  const auto num_values = _prop_values.size();
  if (num_names != num_values)
    paramError("prop_values",
               "Number of vector property names (" + std::to_string(num_names) +
                   ") does not match the number of vectors of property values (" +
                   std::to_string(num_values) + ")");

  _num_props = num_names;
  _properties.resize(num_names);

  for (const auto i : make_range(_num_props))
    _properties[i] = &declareGenericProperty<std::vector<Real>, is_ad>(_prop_names[i]);
}

template <bool is_ad>
void
GenericConstantStdVectorMaterialTempl<is_ad>::initQpStatefulProperties()
{
  computeQpProperties();
}

template <bool is_ad>
void
GenericConstantStdVectorMaterialTempl<is_ad>::computeQpProperties()
{
  for (unsigned int i = 0; i < _num_props; i++)
  {
    auto & prop_out = (*_properties[i])[_qp];
    const auto & prop_in = _prop_values[i];
    prop_out.resize(prop_in.size());
    for (const auto j : index_range(prop_in))
      prop_out[j] = prop_in[j];
  }
}

template <bool is_ad>
std::size_t
GenericConstantStdVectorMaterialTempl<is_ad>::getVectorPropertySize(
    const MaterialPropertyName & prop_name) const
{
  for (const auto i : index_range(_prop_names))
    if (_prop_names[i] == prop_name)
      return _prop_values[i].size();
  paramError("prop_names", "Property '" + prop_name + "' was not defined");
}

template class GenericConstantStdVectorMaterialTempl<false>;
template class GenericConstantStdVectorMaterialTempl<true>;

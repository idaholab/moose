//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GenericConstantVectorMaterial.h"

registerMooseObject("MooseApp", GenericConstantVectorMaterial);
registerMooseObject("MooseApp", ADGenericConstantVectorMaterial);

template <bool is_ad>
InputParameters
GenericConstantVectorMaterialTempl<is_ad>::validParams()
{

  InputParameters params = Material::validParams();
  params.addClassDescription("Declares material properties based on names and vector values "
                             "prescribed by input parameters.");
  params.addRequiredParam<std::vector<std::string>>(
      "prop_names", "The names of the properties this material will have");
  params.addRequiredParam<std::vector<Real>>("prop_values",
                                             "The values associated with the named properties. "
                                             "The vector lengths must be the same.");
  params.declareControllable("prop_values");
  params.set<MooseEnum>("constant_on") = "SUBDOMAIN";
  return params;
}

template <bool is_ad>
GenericConstantVectorMaterialTempl<is_ad>::GenericConstantVectorMaterialTempl(
    const InputParameters & parameters)
  : Material(parameters),
    _prop_names(getParam<std::vector<std::string>>("prop_names")),
    _prop_values(getParam<std::vector<Real>>("prop_values"))
{
  unsigned int num_names = _prop_names.size();
  unsigned int num_values = _prop_values.size();

  if (num_values != num_names * LIBMESH_DIM)
    mooseError("prop_values must be a equal to dim * number of prop_names for a "
               "GenericConstantVectorMaterial.");

  _num_props = num_names;
  _properties.resize(num_names);

  for (unsigned int i = 0; i < _num_props; i++)
    _properties[i] = &declareGenericProperty<RealVectorValue, is_ad>(_prop_names[i]);
}

template <bool is_ad>
void
GenericConstantVectorMaterialTempl<is_ad>::initQpStatefulProperties()
{
  computeQpProperties();
}

template <bool is_ad>
void
GenericConstantVectorMaterialTempl<is_ad>::computeQpProperties()
{
  for (unsigned int i = 0; i < _num_props; i++)
    for (const auto j : make_range(Moose::dim))
      (*_properties[i])[_qp](j) = _prop_values[i * LIBMESH_DIM + j];
}

template class GenericConstantVectorMaterialTempl<false>;
template class GenericConstantVectorMaterialTempl<true>;

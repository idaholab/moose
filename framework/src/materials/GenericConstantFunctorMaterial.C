//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GenericConstantFunctorMaterial.h"

registerMooseObject("MooseApp", GenericConstantFunctorMaterial);
registerMooseObject("MooseApp", ADGenericConstantFunctorMaterial);

template <bool is_ad>
InputParameters
GenericConstantFunctorMaterialTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "Declares material properties based on names and values prescribed by input parameters.");
  params.addRequiredParam<std::vector<std::string>>(
      "prop_names", "The names of the properties this material will have");
  params.addRequiredParam<std::vector<Real>>("prop_values",
                                             "The values associated with the named properties");
  return params;
}

template <bool is_ad>
GenericConstantFunctorMaterialTempl<is_ad>::GenericConstantFunctorMaterialTempl(
    const InputParameters & parameters)
  : Material(parameters),
    _prop_names(getParam<std::vector<std::string>>("prop_names")),
    _prop_values(getParam<std::vector<Real>>("prop_values"))
{
  unsigned int num_names = _prop_names.size();
  unsigned int num_values = _prop_values.size();

  if (num_names != num_values)
    mooseError("Number of prop_names must match the number of prop_values for a "
               "GenericConstantFunctorMaterial!");

  _num_props = num_names;

  for (const auto i : make_range(_num_props))
  {
    auto & prop = declareFunctorProperty<GenericReal<is_ad>>(_prop_names[i]);
    prop.setFunction(_mesh, blockIDs(), [this, i](auto & /*geom_quantity*/) -> GenericReal<is_ad> {
      return _prop_values[i];
    });
  }
}

template class GenericConstantFunctorMaterialTempl<false>;
template class GenericConstantFunctorMaterialTempl<true>;

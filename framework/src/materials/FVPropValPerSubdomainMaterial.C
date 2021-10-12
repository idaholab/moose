//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVPropValPerSubdomainMaterial.h"

registerMooseObject("MooseApp", FVPropValPerSubdomainMaterial);
registerMooseObject("MooseApp", FVADPropValPerSubdomainMaterial);

template <bool is_ad>
InputParameters
FVPropValPerSubdomainMaterialTempl<is_ad>::validParams()
{
  auto params = FunctorMaterial::validParams();
  params.addClassDescription("Computes a property value on a per-subdomain basis");
  // Somehow min gcc doesn't know the type of params here
  params.template addRequiredParam<MaterialPropertyName>("prop_name",
                                                         "The name of the property to declare");
  params.template addRequiredParam<std::map<std::string, Real>>(
      "subdomain_to_prop_value", "Map from subdomain to property value");
  return params;
}

template <bool is_ad>
FVPropValPerSubdomainMaterialTempl<is_ad>::FVPropValPerSubdomainMaterialTempl(
    const InputParameters & params)
  : FunctorMaterial(params), _prop(declareFunctorProperty<GenericReal<is_ad>>("prop_name"))
{
  for (const auto & map_pr : getParam<std::map<std::string, Real>>("subdomain_to_prop_value"))
  {
    const Real value = map_pr.second;
    _prop.setFunctor(
        _mesh,
        {_mesh.getSubdomainID(map_pr.first)},
        [value](const auto & /*r*/, const auto & /*t*/) -> GenericReal<is_ad> { return value; });
  }
}

template class FVPropValPerSubdomainMaterialTempl<false>;
template class FVPropValPerSubdomainMaterialTempl<true>;

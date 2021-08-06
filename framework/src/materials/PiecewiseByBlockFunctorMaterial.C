//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PiecewiseByBlockFunctorMaterial.h"

registerMooseObject("MooseApp", PiecewiseByBlockFunctorMaterial);
registerMooseObject("MooseApp", ADPiecewiseByBlockFunctorMaterial);
registerMooseObjectRenamed("MooseApp",
                           FVPropValPerSubdomainMaterial,
                           "06/30/2022 24:00",
                           PiecewiseByBlockFunctorMaterial);
registerMooseObjectRenamed("MooseApp",
                           FVADPropValPerSubdomainMaterial,
                           "06/30/2022 24:00",
                           ADPiecewiseByBlockFunctorMaterial);

template <bool is_ad>
InputParameters
PiecewiseByBlockFunctorMaterialTempl<is_ad>::validParams()
{
  auto params = FunctorMaterial::validParams();
  params.addClassDescription("Computes a property value on a per-subdomain basis");
  // Somehow min gcc doesn't know the type of params here
  params.template addRequiredParam<MaterialPropertyName>("prop_name",
                                                         "The name of the property to declare");
  params.template addRequiredParam<std::map<std::string, std::string>>(
      "subdomain_to_prop_value",
      "Map from subdomain to property value. The value may be a constant"
      " or any kind of functor (functions, variables, functor material properties)");
  return params;
}

template <bool is_ad>
PiecewiseByBlockFunctorMaterialTempl<is_ad>::PiecewiseByBlockFunctorMaterialTempl(
    const InputParameters & params)
  : FunctorMaterial(params)
{
  for (const auto & map_pr :
       getParam<std::map<std::string, std::string>>("subdomain_to_prop_value"))
  {
    const auto & name = map_pr.second;
    const auto & functor = getFunctor<GenericReal<is_ad>>(name);
    addFunctorPropertyByBlocks<GenericReal<is_ad>>(
        "prop_name",
        [&functor](const auto & r, const auto & t) -> GenericReal<is_ad> { return functor(r, t); },
        std::set<SubdomainID>({_mesh.getSubdomainID(map_pr.first)}));
  }
}

template class PiecewiseByBlockFunctorMaterialTempl<false>;
template class PiecewiseByBlockFunctorMaterialTempl<true>;

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
registerMooseObject("MooseApp", PiecewiseByBlockVectorFunctorMaterial);
registerMooseObject("MooseApp", ADPiecewiseByBlockVectorFunctorMaterial);
registerMooseObjectRenamed("MooseApp",
                           FVPropValPerSubdomainMaterial,
                           "06/30/2022 24:00",
                           PiecewiseByBlockFunctorMaterial);
registerMooseObjectRenamed("MooseApp",
                           FVADPropValPerSubdomainMaterial,
                           "06/30/2022 24:00",
                           ADPiecewiseByBlockFunctorMaterial);

template <typename T>
InputParameters
PiecewiseByBlockFunctorMaterialTempl<T>::validParams()
{
  auto params = FunctorMaterial::validParams();
  params.addClassDescription("Computes a property value on a per-subdomain basis");
  // Somehow min gcc doesn't know the typename of params here
  params.template addRequiredParam<MaterialPropertyName>("prop_name",
                                                         "The name of the property to declare");
  params.template addRequiredParam<std::map<std::string, std::string>>(
      "subdomain_to_prop_value",
      "Map from subdomain to property value. The value may be a constant"
      " or any kind of functor (functions, variables, functor material properties)");
  return params;
}

template <typename T>
PiecewiseByBlockFunctorMaterialTempl<T>::PiecewiseByBlockFunctorMaterialTempl(
    const InputParameters & params)
  : FunctorMaterial(params)
{
  for (const auto & map_pr :
       getParam<std::map<std::string, std::string>>("subdomain_to_prop_value"))
  {
    const auto & name = map_pr.second;
    const auto & functor = getFunctor<T>(name);
    addFunctorPropertyByBlocks<T>(
        "prop_name",
        [&functor](const auto & r, const auto & t) -> T { return functor(r, t); },
        std::set<SubdomainID>({_mesh.getSubdomainID(map_pr.first)}));
  }
}

template class PiecewiseByBlockFunctorMaterialTempl<GenericReal<false>>;
template class PiecewiseByBlockFunctorMaterialTempl<GenericReal<true>>;
template class PiecewiseByBlockFunctorMaterialTempl<GenericRealVectorValue<false>>;
template class PiecewiseByBlockFunctorMaterialTempl<GenericRealVectorValue<true>>;

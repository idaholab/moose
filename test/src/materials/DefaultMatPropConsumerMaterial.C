//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DefaultMatPropConsumerMaterial.h"

registerMooseObject("MooseTestApp", DefaultMatPropConsumerMaterial);
registerMooseObject("MooseTestApp", ADDefaultMatPropConsumerMaterial);

template <bool is_ad>
InputParameters
DefaultMatPropConsumerMaterialTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<std::string>("mat_prop", "prop", "Material property name to fetch");
  return params;
}

template <bool is_ad>
DefaultMatPropConsumerMaterialTempl<is_ad>::DefaultMatPropConsumerMaterialTempl(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _prop_name(getParam<std::string>("mat_prop")),
    _prop(getDefaultMaterialProperty<Real, is_ad>(_prop_name))
{
}

template <bool is_ad>
void
DefaultMatPropConsumerMaterialTempl<is_ad>::computeQpProperties()
{
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "DefaultMatPropConsumerMaterial.h"

template <>
InputParameters
validParams<DefaultMatPropConsumerMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addParam<std::string>("mat_prop", "prop", "Material property name to fetch");
  return params;
}

DefaultMatPropConsumerMaterial::DefaultMatPropConsumerMaterial(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _prop_name(getParam<std::string>("mat_prop")),
    _prop(getDefaultMaterialProperty<Real>(_prop_name))
{
}

void
DefaultMatPropConsumerMaterial::computeQpProperties()
{
}

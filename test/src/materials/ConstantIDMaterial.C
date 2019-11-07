//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConstantIDMaterial.h"

registerMooseObject("MooseTestApp", ConstantIDMaterial);

InputParameters
ConstantIDMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredParam<MaterialPropertyName>("prop_name", "The name of the property");
  params.addRequiredParam<std::vector<Real>>(
      "prop_values", "List of values to be used for property values based on element integer");
  params.addRequiredParam<std::vector<ExtraElementIDName>>("id_name", "The element integer name");

  return params;
}

ConstantIDMaterial::ConstantIDMaterial(const InputParameters & parameters)
  : Material(parameters),
    _id(getElementID("id_name")),
    _prop(declareProperty<Real>(getParam<MaterialPropertyName>("prop_name"))),
    _values(getParam<std::vector<Real>>("prop_values"))
{
}

void
ConstantIDMaterial::computeQpProperties()
{
  _prop[_qp] = _values[_id];
}

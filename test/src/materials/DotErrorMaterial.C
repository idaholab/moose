//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DotErrorMaterial.h"

registerMooseObject("MooseTestApp", DotErrorMaterial);

template <>
InputParameters
validParams<DotErrorMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<std::string>("property_name",
                                       "The desired name for the Material Property.");
  return params;
}

DotErrorMaterial::DotErrorMaterial(const InputParameters & parameters)
  : Material(parameters),
    _prop_name(getParam<std::string>("property_name")),
    _mat_prop(declareProperty<Real>(_prop_name)),
    _mat_prop_dot(getMaterialPropertyDot(_prop_name))
{
}

void
DotErrorMaterial::computeQpProperties()
{
  _mat_prop[_qp] = 0;
}

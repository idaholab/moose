//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MTMaterial.h"

registerMooseObject("MooseTestApp", MTMaterial);

InputParameters
MTMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<Real>("value", 1.0, "lift");
  return params;
}

MTMaterial::MTMaterial(const InputParameters & parameters)
  : Material(parameters), _mat_prop(declareProperty<Real>("matp")), _value(getParam<Real>("value"))
{
}

void
MTMaterial::computeQpProperties()
{
  _mat_prop[_qp] = _q_point[_qp](0) + _value; // x + value
}

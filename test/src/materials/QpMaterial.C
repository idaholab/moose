//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "QpMaterial.h"

registerMooseObject("MooseTestApp", QpMaterial);

InputParameters
QpMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredParam<std::string>("property_name",
                                       "The desired name for the Material Property.");
  return params;
}

QpMaterial::QpMaterial(const InputParameters & parameters)
  : Material(parameters),
    _prop_name(getParam<std::string>("property_name")),
    _mat_prop(declareProperty<Real>(_prop_name))
{
}

void
QpMaterial::computeQpProperties()
{
  _mat_prop[_qp] = _qp;
}

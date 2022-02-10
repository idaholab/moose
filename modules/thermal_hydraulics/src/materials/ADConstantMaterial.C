//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADConstantMaterial.h"

registerMooseObject("ThermalHydraulicsApp", ADConstantMaterial);

InputParameters
ADConstantMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<Real>("value", 0., "Constant value being assigned into the property");
  params.addRequiredParam<std::string>("property_name", "The property name to declare");
  return params;
}

ADConstantMaterial::ADConstantMaterial(const InputParameters & parameters)
  : Material(parameters),
    _value(getParam<Real>("value")),
    _property_name(getParam<std::string>("property_name")),
    _property(declareADProperty<Real>(_property_name))
{
}

void
ADConstantMaterial::computeQpProperties()
{
  _property[_qp] = _value;
}

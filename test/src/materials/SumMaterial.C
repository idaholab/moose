//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SumMaterial.h"

registerMooseObject("MooseTestApp", SumMaterial);

InputParameters
SumMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredParam<MaterialPropertyName>(
      "sum_prop_name", "The name of the property that holds the summation");
  params.addRequiredParam<MaterialPropertyName>(
      "mp1", "The name of the property that holds the first value");
  params.addRequiredParam<MaterialPropertyName>(
      "mp2", "The name of the property that holds the second value");

  params.addRequiredParam<Real>("val1", "The value of the first property");
  params.addRequiredParam<Real>("val2", "The value of the second property");

  return params;
}

SumMaterial::SumMaterial(const InputParameters & parameters)
  : Material(parameters),
    _sum(declareProperty<Real>(getParam<MaterialPropertyName>("sum_prop_name"))),
    _mp1(getMaterialProperty<Real>("mp1")),
    _mp2(getMaterialProperty<Real>("mp2")),
    _val_mp1(getParam<Real>("val1")),
    _val_mp2(getParam<Real>("val2"))
{
}

SumMaterial::~SumMaterial() {}

void
SumMaterial::computeQpProperties()
{
  if (_mp1[_qp] != _val_mp1)
    mooseError("failure");
  if (_mp2[_qp] != _val_mp2)
    mooseError("failure");
  _sum[_qp] = _mp1[_qp] + _mp2[_qp];
}

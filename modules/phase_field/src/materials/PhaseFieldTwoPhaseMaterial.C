//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhaseFieldTwoPhaseMaterial.h"

registerADMooseObject("PhaseFieldApp", PhaseFieldTwoPhaseMaterial);

InputParameters
PhaseFieldTwoPhaseMaterial::validParams()
{
  InputParameters params = ADMaterial::validParams();
  params.addRequiredCoupledVar("pf", "Phase field variable");
  params.addRequiredParam<MaterialPropertyName>("prop_name", "Name of material property.");
  params.addRequiredParam<Real>("prop_value_1", "value of phase 1.");
  params.addRequiredParam<Real>("prop_value_2", "value of phase 2.");
  return params;
}

PhaseFieldTwoPhaseMaterial::PhaseFieldTwoPhaseMaterial(const InputParameters & parameters)
  : ADMaterial(parameters),
    _pf(adCoupledValue("pf")),
    _prop_value_1(getParam<Real>("prop_value_1")),
    _prop_value_2(getParam<Real>("prop_value_2")),
    _prop(declareADProperty<Real>(getParam<MaterialPropertyName>("prop_name")))
{
}

void
PhaseFieldTwoPhaseMaterial::computeQpProperties()
{
  _prop[_qp] = 0.5 * (1 - _pf[_qp]) * _prop_value_1 + 0.5 * (1 + _pf[_qp]) * _prop_value_2;
}

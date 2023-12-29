//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TensorMechanicsHardeningPowerRule.h"

registerMooseObject("TensorMechanicsApp", TensorMechanicsHardeningPowerRule);

InputParameters
TensorMechanicsHardeningPowerRule::validParams()
{
  InputParameters params = TensorMechanicsHardeningModel::validParams();
  params.addRequiredParam<Real>("value_0", "The yield strength when internal variable = 0");
  params.addParam<Real>("epsilon0", 1.0, "The reference strain");
  params.addParam<Real>(
      "exponent",
      0.0,
      "Let p = internal_parameter.  Then value = value_0 * (p / epsilon0 + 1)^{exponent})");
  params.addClassDescription("Hardening defined by power rule");
  return params;
}

TensorMechanicsHardeningPowerRule::TensorMechanicsHardeningPowerRule(
    const InputParameters & parameters)
  : TensorMechanicsHardeningModel(parameters),
    _value_0(getParam<Real>("value_0")),
    _epsilon0(getParam<Real>("epsilon0")),
    _exponent(getParam<Real>("exponent"))
{
}

Real
TensorMechanicsHardeningPowerRule::value(Real intnl) const
{
  return _value_0 * std::pow(intnl / _epsilon0 + 1, _exponent);
}

Real
TensorMechanicsHardeningPowerRule::derivative(Real intnl) const
{
  return _value_0 * _exponent / _epsilon0 * std::pow(intnl / _epsilon0 + 1, _exponent - 1);
}

std::string
TensorMechanicsHardeningPowerRule::modelName() const
{
  return "PowerRule";
}

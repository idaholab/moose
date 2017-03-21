/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "TensorMechanicsHardeningPowerRule.h"

template <>
InputParameters
validParams<TensorMechanicsHardeningPowerRule>()
{
  InputParameters params = validParams<TensorMechanicsHardeningModel>();
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

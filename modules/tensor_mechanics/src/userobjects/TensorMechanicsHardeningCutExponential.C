/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "TensorMechanicsHardeningCutExponential.h"

template <>
InputParameters
validParams<TensorMechanicsHardeningCutExponential>()
{
  InputParameters params = validParams<TensorMechanicsHardeningModel>();
  params.addRequiredParam<Real>(
      "value_0", "The value of the parameter for all internal_parameter <= internal_0");
  params.addParam<Real>("value_residual",
                        "The value of the parameter for internal_parameter = "
                        "infinity.  Default = value_0, ie perfect plasticity");
  params.addParam<Real>("internal_0", 0, "The cutoff of internal parameter");
  params.addParam<Real>("rate",
                        0,
                        "Let p = internal_parameter.  Then value = value_0 for "
                        "p<internal_0, and otherwise, value = value_residual + (value_0 "
                        "- value_residual)*exp(-rate*(p - internal_0)");
  params.addClassDescription("Hardening is Cut-exponential");
  return params;
}

TensorMechanicsHardeningCutExponential::TensorMechanicsHardeningCutExponential(
    const InputParameters & parameters)
  : TensorMechanicsHardeningModel(parameters),
    _val_0(getParam<Real>("value_0")),
    _val_res(parameters.isParamValid("value_residual") ? getParam<Real>("value_residual") : _val_0),
    _intnl_0(getParam<Real>("internal_0")),
    _rate(getParam<Real>("rate"))
{
}

Real
TensorMechanicsHardeningCutExponential::value(Real intnl) const
{
  Real x = intnl - _intnl_0;
  if (x <= 0)
    return _val_0;
  else
    return _val_res + (_val_0 - _val_res) * std::exp(-_rate * x);
}

Real
TensorMechanicsHardeningCutExponential::derivative(Real intnl) const
{
  Real x = intnl - _intnl_0;
  if (x <= 0)
    return 0;
  else
    return -_rate * (_val_0 - _val_res) * std::exp(-_rate * x);
}

std::string
TensorMechanicsHardeningCutExponential::modelName() const
{
  return "CutExponential";
}

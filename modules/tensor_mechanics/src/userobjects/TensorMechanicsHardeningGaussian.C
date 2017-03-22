/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "TensorMechanicsHardeningGaussian.h"

template <>
InputParameters
validParams<TensorMechanicsHardeningGaussian>()
{
  InputParameters params = validParams<TensorMechanicsHardeningModel>();
  params.addRequiredParam<Real>(
      "value_0", "The value of the parameter for all internal_parameter <= internal_0");
  params.addParam<Real>("value_residual",
                        "The value of the parameter for internal_parameter = "
                        "infinity.  Default = value_0, ie perfect plasticity");
  params.addParam<Real>(
      "internal_0", 0, "The value of the internal_parameter when hardening begins");
  params.addParam<Real>("rate",
                        0,
                        "Let p = internal_parameter.  Then value = value_0 for "
                        "p<internal_0, and value = value_residual + (value_0 - "
                        "value_residual)*exp(-0.5*rate*(p - internal_0)^2)");
  params.addClassDescription("Hardening is Gaussian");
  return params;
}

TensorMechanicsHardeningGaussian::TensorMechanicsHardeningGaussian(
    const InputParameters & parameters)
  : TensorMechanicsHardeningModel(parameters),
    _val_0(getParam<Real>("value_0")),
    _val_res(parameters.isParamValid("value_residual") ? getParam<Real>("value_residual") : _val_0),
    _intnl_0(getParam<Real>("internal_0")),
    _rate(getParam<Real>("rate"))
{
}

Real
TensorMechanicsHardeningGaussian::value(Real intnl) const
{
  Real x = intnl - _intnl_0;
  if (x <= 0)
    return _val_0;
  else
    return _val_res + (_val_0 - _val_res) * std::exp(-0.5 * _rate * x * x);
}

Real
TensorMechanicsHardeningGaussian::derivative(Real intnl) const
{
  Real x = intnl - _intnl_0;
  if (x <= 0)
    return 0;
  else
    return -_rate * x * (_val_0 - _val_res) * std::exp(-0.5 * _rate * x * x);
}

std::string
TensorMechanicsHardeningGaussian::modelName() const
{
  return "Gaussian";
}

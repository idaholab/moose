/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "TensorMechanicsHardeningExponential.h"

template <>
InputParameters
validParams<TensorMechanicsHardeningExponential>()
{
  InputParameters params = validParams<TensorMechanicsHardeningModel>();
  params.addRequiredParam<Real>("value_0", "The value of the parameter at internal_parameter = 0");
  params.addParam<Real>("value_residual",
                        "The value of the parameter for internal_parameter = "
                        "infinity.  Default = value_0, ie perfect plasticity");
  params.addParam<Real>("rate",
                        0,
                        "Let p = internal_parameter.  Then value = value_residual + "
                        "(value_0 - value_residual)*exp(-rate*intnal_parameter)");
  params.addClassDescription("Hardening is Exponential");
  return params;
}

TensorMechanicsHardeningExponential::TensorMechanicsHardeningExponential(
    const InputParameters & parameters)
  : TensorMechanicsHardeningModel(parameters),
    _val_0(getParam<Real>("value_0")),
    _val_res(parameters.isParamValid("value_residual") ? getParam<Real>("value_residual") : _val_0),
    _rate(getParam<Real>("rate"))
{
}

Real
TensorMechanicsHardeningExponential::value(Real intnl) const
{
  return _val_res + (_val_0 - _val_res) * std::exp(-_rate * intnl);
}

Real
TensorMechanicsHardeningExponential::derivative(Real intnl) const
{
  return -_rate * (_val_0 - _val_res) * std::exp(-_rate * intnl);
}

std::string
TensorMechanicsHardeningExponential::modelName() const
{
  return "Exponential";
}

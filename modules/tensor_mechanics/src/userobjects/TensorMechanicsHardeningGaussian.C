//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TensorMechanicsHardeningGaussian.h"

registerMooseObject("TensorMechanicsApp", TensorMechanicsHardeningGaussian);

InputParameters
TensorMechanicsHardeningGaussian::validParams()
{
  InputParameters params = TensorMechanicsHardeningModel::validParams();
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

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TensorMechanicsHardeningCutExponential.h"

registerMooseObject("TensorMechanicsApp", TensorMechanicsHardeningCutExponential);

InputParameters
TensorMechanicsHardeningCutExponential::validParams()
{
  InputParameters params = TensorMechanicsHardeningModel::validParams();
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

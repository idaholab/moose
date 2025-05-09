//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolidMechanicsHardeningExponential.h"

registerMooseObject("SolidMechanicsApp", SolidMechanicsHardeningExponential);
registerMooseObjectRenamed("SolidMechanicsApp",
                           TensorMechanicsHardeningExponential,
                           "01/01/2025 00:00",
                           SolidMechanicsHardeningExponential);

InputParameters
SolidMechanicsHardeningExponential::validParams()
{
  InputParameters params = SolidMechanicsHardeningModel::validParams();
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

SolidMechanicsHardeningExponential::SolidMechanicsHardeningExponential(
    const InputParameters & parameters)
  : SolidMechanicsHardeningModel(parameters),
    _val_0(getParam<Real>("value_0")),
    _val_res(parameters.isParamValid("value_residual") ? getParam<Real>("value_residual") : _val_0),
    _rate(getParam<Real>("rate"))
{
}

Real
SolidMechanicsHardeningExponential::value(Real intnl) const
{
  return _val_res + (_val_0 - _val_res) * std::exp(-_rate * intnl);
}

Real
SolidMechanicsHardeningExponential::derivative(Real intnl) const
{
  return -_rate * (_val_0 - _val_res) * std::exp(-_rate * intnl);
}

std::string
SolidMechanicsHardeningExponential::modelName() const
{
  return "Exponential";
}

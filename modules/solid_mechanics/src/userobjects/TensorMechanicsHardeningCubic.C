//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TensorMechanicsHardeningCubic.h"
#include "libmesh/utility.h"

registerMooseObject("TensorMechanicsApp", TensorMechanicsHardeningCubic);

InputParameters
TensorMechanicsHardeningCubic::validParams()
{
  InputParameters params = TensorMechanicsHardeningModel::validParams();
  params.addRequiredParam<Real>(
      "value_0", "The value of the parameter for all internal_parameter <= internal_0");
  params.addParam<Real>("value_residual",
                        "The value of the parameter for internal_parameter >= "
                        "internal_limit.  Default = value_0, ie perfect "
                        "plasticity");
  params.addParam<Real>(
      "internal_0", 0.0, "The value of the internal_parameter when hardening begins");
  params.addParam<Real>("internal_limit",
                        1.0,
                        "The value of the internal_parameter when hardening "
                        "ends.  This hardening forms a cubic between "
                        "(internal_0, value_0) and (internal_limit, "
                        "value_residual) that is smooth at internal_0 and "
                        "internal_limit");
  params.addClassDescription("Hardening is Cubic");
  return params;
}

TensorMechanicsHardeningCubic::TensorMechanicsHardeningCubic(const InputParameters & parameters)
  : TensorMechanicsHardeningModel(parameters),
    _val_0(getParam<Real>("value_0")),
    _val_res(parameters.isParamValid("value_residual") ? getParam<Real>("value_residual") : _val_0),
    _intnl_0(getParam<Real>("internal_0")),
    _intnl_limit(getParam<Real>("internal_limit")),
    _half_intnl_limit(0.5 * (_intnl_limit - _intnl_0)),
    _alpha((_val_0 - _val_res) / (4.0 * Utility::pow<3>(_half_intnl_limit))),
    _beta(-3.0 * _alpha * Utility::pow<2>(_half_intnl_limit))
{
  if (_intnl_limit <= _intnl_0)
    mooseError("internal_limit must be greater than internal_0 in Cubic Hardening");
}

Real
TensorMechanicsHardeningCubic::value(Real intnl) const
{
  const Real x = intnl - _intnl_0;
  if (x <= 0.0)
    return _val_0;
  else if (intnl >= _intnl_limit)
    return _val_res;
  else
    return _alpha * Utility::pow<3>(x - _half_intnl_limit) + _beta * (x - _half_intnl_limit) +
           0.5 * (_val_0 + _val_res);
}

Real
TensorMechanicsHardeningCubic::derivative(Real intnl) const
{
  const Real x = intnl - _intnl_0;
  if (x <= 0.0)
    return 0.0;
  else if (intnl >= _intnl_limit)
    return 0.0;
  else
    return 3.0 * _alpha * Utility::pow<2>(x - _half_intnl_limit) + _beta;
}

std::string
TensorMechanicsHardeningCubic::modelName() const
{
  return "Cubic";
}

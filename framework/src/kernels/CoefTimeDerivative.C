//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoefTimeDerivative.h"

registerMooseObject("MooseApp", CoefTimeDerivative);

template <>
InputParameters
validParams<CoefTimeDerivative>()
{
  InputParameters params = validParams<TimeDerivative>();
  params.addParam<Real>("Coefficient", 1, "The coefficient for the time derivative kernel");
  params.addParam<MaterialPropertyName>(
      "mat_prop_coef", 1.0, "Material property coefficient for the time derivative kernel");
  params.addParam<bool>(
      "prop_in_derivative", false, "Whether or not the property is inside time derivative");
  return params;
}

CoefTimeDerivative::CoefTimeDerivative(const InputParameters & parameters)
  : TimeDerivative(parameters),
    _coef(getParam<Real>("Coefficient")),
    _prop_coef(getMaterialProperty<Real>("mat_prop_coef"))
{
  if (getParam<bool>("prop_in_derivative"))
    _prop_dot = &getMaterialPropertyDot<Real>("mat_prop_coef");
  else
    _prop_dot = nullptr;
}

Real
CoefTimeDerivative::computeQpResidual()
{
  if (_prop_dot)
    return _test[_i][_qp] * _coef * (_prop_coef[_qp] * _u_dot[_qp] + (*_prop_dot)[_qp] * _u[_qp]);
  else
    return _test[_i][_qp] * _coef * _prop_coef[_qp] * _u_dot[_qp];
}

Real
CoefTimeDerivative::computeQpJacobian()
{
  if (_prop_dot)
    return _test[_i][_qp] * _coef * _phi[_j][_qp] *
           (_prop_coef[_qp] * _du_dot_du[_qp] + (*_prop_dot)[_qp]);
  else
    return _test[_i][_qp] * _coef * _phi[_j][_qp] * _prop_coef[_qp] * _du_dot_du[_qp];
}

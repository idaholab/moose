//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PHarmonic.h"

registerMooseObject("MooseTestApp", PHarmonic);

InputParameters
PHarmonic::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addRangeCheckedParam<Real>("p", 2.0, "p>=1.0", "The exponent p");
  return params;
}

PHarmonic::PHarmonic(const InputParameters & parameters)
  : Kernel(parameters), _p(getParam<Real>("p") - 2.0)
{
}

Real
PHarmonic::computeQpResidual()
{
  Real t = std::sqrt(_grad_u[_qp] * _grad_u[_qp]);
  return _grad_test[_i][_qp] * _grad_u[_qp] * std::pow(t, _p);
}

Real
PHarmonic::computeQpJacobian()
{
  // Note: this jacobian evaluation is not exact when p!=2.
  Real t = std::sqrt(_grad_phi[_j][_qp] * _grad_phi[_j][_qp]);
  return _grad_test[_i][_qp] * _grad_phi[_j][_qp] * std::pow(t, _p);
}

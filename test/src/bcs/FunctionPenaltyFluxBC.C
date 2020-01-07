//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionPenaltyFluxBC.h"
#include "Function.h"

registerMooseObject("MooseTestApp", FunctionPenaltyFluxBC);

InputParameters
FunctionPenaltyFluxBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addRequiredParam<Real>("penalty", "Penalty scalar");
  params.addRequiredParam<FunctionName>("function",
                                        "Function used to compute the desired normal flux");
  return params;
}

FunctionPenaltyFluxBC::FunctionPenaltyFluxBC(const InputParameters & parameters)
  : IntegratedBC(parameters), _func(getFunction("function")), _p(getParam<Real>("penalty"))
{
}

Real
FunctionPenaltyFluxBC::computeQpResidual()
{
  Real dudn = _grad_u[_qp] * _normals[_qp];
  Real dgdn = _func.gradient(_t, _q_point[_qp]) * _normals[_qp];
  Real dvdn = _grad_test[_i][_qp] * _normals[_qp];
  return _p * (dudn - dgdn) * dvdn;
}

Real
FunctionPenaltyFluxBC::computeQpJacobian()
{
  Real dphi_j_dn = _grad_phi[_j][_qp] * _normals[_qp];
  Real dphi_i_dn = _grad_test[_i][_qp] * _normals[_qp];

  return _p * dphi_j_dn * dphi_i_dn;
}

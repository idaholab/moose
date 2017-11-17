/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
#include "FunctionPenaltyFluxBC.h"
#include "Function.h"

template <>
InputParameters
validParams<FunctionPenaltyFluxBC>()
{
  InputParameters params = validParams<IntegratedBC>();
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

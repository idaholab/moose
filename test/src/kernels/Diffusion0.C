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
#include "Diffusion0.h"

template <>
InputParameters
validParams<Diffusion0>()
{
  InputParameters params = validParams<Kernel>();

  params.set<Real>("Ak") = 1.0;
  params.set<Real>("Bk") = 1.0;
  params.set<Real>("Ck") = 1.0;

  return params;
}

Diffusion0::Diffusion0(const InputParameters & parameters) : Kernel(parameters)
{
  _Ak = getParam<Real>("Ak");
  _Bk = getParam<Real>("Bk");
  _Ck = getParam<Real>("Ck");
}

Real
Diffusion0::computeQpResidual()
{
  Real diff = _Ak + _Bk * _q_point[_qp](0) + _Ck * _q_point[_qp](1);

  return diff * _grad_test[_i][_qp] * _grad_u[_qp];
}

Real
Diffusion0::computeQpJacobian()
{
  Real diff = _Ak + _Bk * _q_point[_qp](0) + _Ck * _q_point[_qp](1);

  return diff * _grad_test[_i][_qp] * _grad_phi[_j][_qp];
}

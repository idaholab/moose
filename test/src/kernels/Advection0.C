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
#include "Advection0.h"

template <>
InputParameters
validParams<Advection0>()
{
  InputParameters params = validParams<Kernel>();

  params.set<Real>("Au") = 1.0;
  params.set<Real>("Bu") = 1.0;
  params.set<Real>("Cu") = 1.0;

  params.set<Real>("Av") = 1.0;
  params.set<Real>("Bv") = 1.0;
  params.set<Real>("Cv") = 1.0;

  return params;
}

Advection0::Advection0(const InputParameters & parameters) : Kernel(parameters)
{
  _Au = getParam<Real>("Au");
  _Bu = getParam<Real>("Bu");
  _Cu = getParam<Real>("Cu");

  _Av = getParam<Real>("Av");
  _Bv = getParam<Real>("Bv");
  _Cv = getParam<Real>("Cv");
}

Real
Advection0::computeQpResidual()
{
  VectorValue<Number> vel(_Au + _Bu * _q_point[_qp](0) + _Cu * _q_point[_qp](1),
                          _Av + _Bv * _q_point[_qp](0) + _Cv * _q_point[_qp](1),
                          0.0);
  return -_test[_i][_qp] * vel * _grad_u[_qp];
}

Real
Advection0::computeQpJacobian()
{
  VectorValue<Number> vel(_Au + _Bu * _q_point[_qp](0) + _Cu * _q_point[_qp](1),
                          _Av + _Bv * _q_point[_qp](0) + _Cv * _q_point[_qp](1),
                          0.0);
  return -_test[_i][_qp] * vel * _grad_phi[_j][_qp];
}

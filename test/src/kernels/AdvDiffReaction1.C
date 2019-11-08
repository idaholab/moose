//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdvDiffReaction1.h"

registerMooseObject("MooseTestApp", AdvDiffReaction1);

InputParameters
AdvDiffReaction1::validParams()
{
  InputParameters params = Kernel::validParams();

  params.set<Real>("A0") = 0.0;
  params.set<Real>("B0") = 0.0;
  params.set<Real>("C0") = 0.0;

  params.set<Real>("Au") = 1.0;
  params.set<Real>("Bu") = 1.0;
  params.set<Real>("Cu") = 1.0;

  params.set<Real>("Av") = 1.0;
  params.set<Real>("Bv") = 1.0;
  params.set<Real>("Cv") = 1.0;

  params.set<Real>("Ak") = 1.0;
  params.set<Real>("Bk") = 1.0;
  params.set<Real>("Ck") = 1.0;

  params.set<Real>("omega0") = 1.0;

  return params;
}

AdvDiffReaction1::AdvDiffReaction1(const InputParameters & parameters) : Kernel(parameters)
{
  _A0 = getParam<Real>("A0");
  _B0 = getParam<Real>("B0");
  _C0 = getParam<Real>("C0");

  _Au = getParam<Real>("Au");
  _Bu = getParam<Real>("Bu");
  _Cu = getParam<Real>("Cu");

  _Av = getParam<Real>("Av");
  _Bv = getParam<Real>("Bv");
  _Cv = getParam<Real>("Cv");

  _Ak = getParam<Real>("Ak");
  _Bk = getParam<Real>("Bk");
  _Ck = getParam<Real>("Ck");

  _omega0 = getParam<Real>("omega0");
}

Real
AdvDiffReaction1::computeQpResidual()
{
  // Reaction:
  Real qpResidual =
      _test[_i][_qp] *
      ManSol4ADR2src(
          _q_point[_qp], _A0, _B0, _C0, _Au, _Bu, _Cu, _Av, _Bv, _Cv, _Ak, _Bk, _Ck, _omega0, _t);
  // Advection:
  VectorValue<Number> vel(_Au + _Bu * _q_point[_qp](0) + _Cu * _q_point[_qp](1),
                          _Av + _Bv * _q_point[_qp](0) + _Cv * _q_point[_qp](1),
                          0.0);
  qpResidual += -_test[_i][_qp] * vel * _grad_u[_qp];

  // Diffusion:
  Real diff = _Ak + _Bk * _q_point[_qp](0) + _Ck * _q_point[_qp](1);
  qpResidual += diff * _grad_test[_i][_qp] * _grad_u[_qp];

  return qpResidual;
}

Real
AdvDiffReaction1::computeQpJacobian()
{
  // Advection:
  VectorValue<Number> vel(_Au + _Bu * _q_point[_qp](0) + _Cu * _q_point[_qp](1),
                          _Av + _Bv * _q_point[_qp](0) + _Cv * _q_point[_qp](1),
                          0.0);
  Real jacob = -_test[_i][_qp] * vel * _grad_phi[_j][_qp];

  // Diffusion:
  Real diff = _Ak + _Bk * _q_point[_qp](0) + _Ck * _q_point[_qp](1);
  jacob += diff * _grad_test[_i][_qp] * _grad_phi[_j][_qp];

  return jacob;
}

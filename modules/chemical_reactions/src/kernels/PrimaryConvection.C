/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PrimaryConvection.h"

template <>
InputParameters
validParams<PrimaryConvection>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredCoupledVar("p", "Pressure");
  return params;
}

PrimaryConvection::PrimaryConvection(const InputParameters & parameters)
  : Kernel(parameters),
    _cond(getMaterialProperty<Real>("conductivity")),
    _grad_p(coupledGradient("p"))
{
}

Real
PrimaryConvection::computeQpResidual()
{
  RealGradient _Darcy_vel = -_grad_p[_qp] * _cond[_qp];

  return _test[_i][_qp] * (_Darcy_vel * _grad_u[_qp]);
}

Real
PrimaryConvection::computeQpJacobian()
{
  RealGradient _Darcy_vel = -_grad_p[_qp] * _cond[_qp];

  return _test[_i][_qp] * (_Darcy_vel * _grad_phi[_j][_qp]);
}

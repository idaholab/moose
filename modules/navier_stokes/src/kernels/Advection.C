/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "Advection.h"

template <>
InputParameters
validParams<Advection>()
{
  InputParameters params = validParams<INSBase>();

  params.addClassDescription("Advection term.");
  return params;
}

Advection::Advection(const InputParameters & parameters) : INSBase(parameters) {}

Real
Advection::computeQpResidual()
{
  RealVectorValue(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
  return _test[_i][_qp] * RealVectorValue(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]) * _grad_u[_qp];
}

Real
Advection::computeQpJacobian()
{
  RealVectorValue U(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
  return _test[_i][_qp] * RealVectorValue(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]) *
         _grad_phi[_j][_qp];
}

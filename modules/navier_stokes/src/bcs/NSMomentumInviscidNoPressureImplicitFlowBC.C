/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "NSMomentumInviscidNoPressureImplicitFlowBC.h"

template <>
InputParameters
validParams<NSMomentumInviscidNoPressureImplicitFlowBC>()
{
  InputParameters params = validParams<NSMomentumInviscidBC>();
  params.addClassDescription(
      "Momentum equation boundary condition used when pressure *is not* integrated by parts.");
  return params;
}

NSMomentumInviscidNoPressureImplicitFlowBC::NSMomentumInviscidNoPressureImplicitFlowBC(
    const InputParameters & parameters)
  : NSMomentumInviscidBC(parameters)
{
}

Real
NSMomentumInviscidNoPressureImplicitFlowBC::computeQpResidual()
{
  // Velocity vector object
  RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  // Velocity vector dotted with normal
  Real u_dot_n = vel * _normals[_qp];

  // The current value of the vector (rho*u)(u.n)
  RealVectorValue rhou_udotn = u_dot_n * _rho[_qp] * vel;

  return convectiveQpResidualHelper(rhou_udotn(_component));
}

Real
NSMomentumInviscidNoPressureImplicitFlowBC::computeQpJacobian()
{
  // There is no Jacobian for the pressure term when the pressure is specified,
  // so all we have left is the convective part.  The on-diagonal variable number
  // is _component+1
  return convectiveQpJacobianHelper(_component + 1);
}

Real
NSMomentumInviscidNoPressureImplicitFlowBC::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (isNSVariable(jvar))
    return convectiveQpJacobianHelper(mapVarNumber(jvar));
  else
    return 0.0;
}

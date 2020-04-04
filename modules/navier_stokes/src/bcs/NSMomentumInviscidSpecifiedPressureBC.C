//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSMomentumInviscidSpecifiedPressureBC.h"

registerMooseObject("NavierStokesApp", NSMomentumInviscidSpecifiedPressureBC);

InputParameters
NSMomentumInviscidSpecifiedPressureBC::validParams()
{
  InputParameters params = NSMomentumInviscidBC::validParams();
  params.addClassDescription("Momentum equation boundary condition in which pressure is specified "
                             "(given) and the value of the convective part is allowed to vary (is "
                             "computed implicitly).");
  return params;
}

NSMomentumInviscidSpecifiedPressureBC::NSMomentumInviscidSpecifiedPressureBC(
    const InputParameters & parameters)
  : NSMomentumInviscidBC(parameters), _specified_pressure(getParam<Real>("specified_pressure"))
{
}

Real
NSMomentumInviscidSpecifiedPressureBC::computeQpResidual()
{
  // Velocity vector object
  RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  // Velocity vector dotted with normal
  Real u_dot_n = vel * _normals[_qp];

  // The current value of the vector (rho*u)(u.n)
  RealVectorValue rhou_udotn = u_dot_n * _rho[_qp] * vel;

  return pressureQpResidualHelper(_specified_pressure) +
         convectiveQpResidualHelper(rhou_udotn(_component));
}

Real
NSMomentumInviscidSpecifiedPressureBC::computeQpJacobian()
{
  // There is no Jacobian for the pressure term when the pressure is specified,
  // so all we have left is the convective part.  The on-diagonal variable number
  // is _component+1
  return convectiveQpJacobianHelper(_component + 1);
}

Real
NSMomentumInviscidSpecifiedPressureBC::computeQpOffDiagJacobian(unsigned jvar)
{
  if (isNSVariable(jvar))
    return convectiveQpJacobianHelper(mapVarNumber(jvar));
  else
    return 0.0;
}

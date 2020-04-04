//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSEnergyInviscidSpecifiedPressureBC.h"

registerMooseObject("NavierStokesApp", NSEnergyInviscidSpecifiedPressureBC);

InputParameters
NSEnergyInviscidSpecifiedPressureBC::validParams()
{
  InputParameters params = NSEnergyInviscidBC::validParams();
  return params;
}

NSEnergyInviscidSpecifiedPressureBC::NSEnergyInviscidSpecifiedPressureBC(
    const InputParameters & parameters)
  : NSEnergyInviscidBC(parameters), _specified_pressure(getParam<Real>("specified_pressure"))
{
}

Real
NSEnergyInviscidSpecifiedPressureBC::computeQpResidual()
{
  // Velocity vector object
  RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  // Normal component
  Real un = vel * _normals[_qp];

  return qpResidualHelper(_specified_pressure, un);
}

Real
NSEnergyInviscidSpecifiedPressureBC::computeQpJacobian()
{
  return computeJacobianHelper(/*on-diagonal variable is energy=*/4);
}

Real
NSEnergyInviscidSpecifiedPressureBC::computeQpOffDiagJacobian(unsigned jvar)
{
  if (isNSVariable(jvar))
    return computeJacobianHelper(mapVarNumber(jvar));
  else
    return 0.0;
}

Real
NSEnergyInviscidSpecifiedPressureBC::computeJacobianHelper(unsigned var_number)
{
  // Velocity vector object
  RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  // Normal component of velocity
  Real un = vel * _normals[_qp];

  // For specified pressure, term "C" is zero, see base class for details.
  return qpJacobianTermA(var_number, _specified_pressure) + qpJacobianTermB(var_number, un);
}

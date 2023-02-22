//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes includes
#include "NS.h"
#include "NSEnergyInviscidUnspecifiedBC.h"

registerMooseObject("NavierStokesApp", NSEnergyInviscidUnspecifiedBC);

InputParameters
NSEnergyInviscidUnspecifiedBC::validParams()
{
  InputParameters params = NSEnergyInviscidBC::validParams();
  params.addRequiredCoupledVar(NS::pressure, "pressure");
  return params;
}

NSEnergyInviscidUnspecifiedBC::NSEnergyInviscidUnspecifiedBC(const InputParameters & parameters)
  : NSEnergyInviscidBC(parameters), _pressure(coupledValue(NS::pressure))
{
}

Real
NSEnergyInviscidUnspecifiedBC::computeQpResidual()
{
  // Velocity vector object
  RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  // Normal component
  Real un = vel * _normals[_qp];

  return qpResidualHelper(_pressure[_qp], un);
}

Real
NSEnergyInviscidUnspecifiedBC::computeQpJacobian()
{
  return computeJacobianHelper(/*on-diagonal variable is energy=*/4);
}

Real
NSEnergyInviscidUnspecifiedBC::computeQpOffDiagJacobian(unsigned jvar)
{
  if (isNSVariable(jvar))
    return computeJacobianHelper(mapVarNumber(jvar));
  else
    return 0.0;
}

Real
NSEnergyInviscidUnspecifiedBC::computeJacobianHelper(unsigned var_number)
{
  // Velocity vector object
  RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  // Normal component of velocity
  Real un = vel * _normals[_qp];

  // When both u.n and pressure are unspecified, all 3 Jacobian terms apply.
  // See base class for details.
  return qpJacobianTermA(var_number, _pressure[_qp]) + qpJacobianTermB(var_number, un) +
         qpJacobianTermC(var_number, un);
}

/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

// Navier-Stokes includes
#include "NS.h"
#include "NSEnergyInviscidUnspecifiedBC.h"

template <>
InputParameters
validParams<NSEnergyInviscidUnspecifiedBC>()
{
  InputParameters params = validParams<NSEnergyInviscidBC>();
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

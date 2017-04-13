/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "NSEnergyInviscidSpecifiedPressureBC.h"

template <>
InputParameters
validParams<NSEnergyInviscidSpecifiedPressureBC>()
{
  InputParameters params = validParams<NSEnergyInviscidBC>();
  params.addRequiredParam<Real>("specified_pressure", "The specified pressure for this boundary");
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

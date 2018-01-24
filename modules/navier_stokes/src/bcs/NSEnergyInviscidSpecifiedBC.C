/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "NSEnergyInviscidSpecifiedBC.h"

template <>
InputParameters
validParams<NSEnergyInviscidSpecifiedBC>()
{
  InputParameters params = validParams<NSEnergyInviscidBC>();
  params.addRequiredParam<Real>("specified_pressure", "The specified pressure for this boundary");
  params.addRequiredParam<Real>("un", "The specified value of u.n for this boundary");
  return params;
}

NSEnergyInviscidSpecifiedBC::NSEnergyInviscidSpecifiedBC(const InputParameters & parameters)
  : NSEnergyInviscidBC(parameters),
    _specified_pressure(getParam<Real>("specified_pressure")),
    _un(getParam<Real>("un"))
{
}

Real
NSEnergyInviscidSpecifiedBC::computeQpResidual()
{
  return qpResidualHelper(_specified_pressure, _un);
}

Real
NSEnergyInviscidSpecifiedBC::computeQpJacobian()
{
  return this->computeJacobianHelper(/*on-diagonal variable is energy=*/4);
}

Real
NSEnergyInviscidSpecifiedBC::computeQpOffDiagJacobian(unsigned jvar)
{
  if (isNSVariable(jvar))
    return computeJacobianHelper(mapVarNumber(jvar));
  else
    return 0.0;
}

Real
NSEnergyInviscidSpecifiedBC::computeJacobianHelper(unsigned var_number)
{
  // When both pressure and u.n are specified, only term B of the Jacobian is non-zero.
  return qpJacobianTermB(var_number, _un);
}

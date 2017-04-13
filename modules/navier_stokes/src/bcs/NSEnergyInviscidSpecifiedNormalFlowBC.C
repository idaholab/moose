/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

// Navier-Stokes includes
#include "NS.h"
#include "NSEnergyInviscidSpecifiedNormalFlowBC.h"

template <>
InputParameters
validParams<NSEnergyInviscidSpecifiedNormalFlowBC>()
{
  InputParameters params = validParams<NSEnergyInviscidBC>();
  params.addRequiredCoupledVar(NS::pressure, "pressure");
  params.addRequiredParam<Real>("un", "The specified value of u.n for this boundary");
  return params;
}

NSEnergyInviscidSpecifiedNormalFlowBC::NSEnergyInviscidSpecifiedNormalFlowBC(
    const InputParameters & parameters)
  : NSEnergyInviscidBC(parameters), _pressure(coupledValue(NS::pressure)), _un(getParam<Real>("un"))
{
}

Real
NSEnergyInviscidSpecifiedNormalFlowBC::computeQpResidual()
{
  return qpResidualHelper(_pressure[_qp], _un);
}

Real
NSEnergyInviscidSpecifiedNormalFlowBC::computeQpJacobian()
{
  return computeJacobianHelper(/*on-diagonal variable is energy=*/4);
}

Real
NSEnergyInviscidSpecifiedNormalFlowBC::computeQpOffDiagJacobian(unsigned jvar)
{
  if (isNSVariable(jvar))
    return computeJacobianHelper(mapVarNumber(jvar));
  else
    return 0.0;
}

Real
NSEnergyInviscidSpecifiedNormalFlowBC::computeJacobianHelper(unsigned var_number)
{
  // For specified u.n, term "A" is zero, see base class for details.
  return qpJacobianTermB(var_number, _un) + qpJacobianTermC(var_number, _un);
}

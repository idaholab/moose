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
#include "NSEnergyInviscidSpecifiedNormalFlowBC.h"

registerMooseObject("NavierStokesApp", NSEnergyInviscidSpecifiedNormalFlowBC);

InputParameters
NSEnergyInviscidSpecifiedNormalFlowBC::validParams()
{
  InputParameters params = NSEnergyInviscidBC::validParams();
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

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
#include "NSMomentumInviscidSpecifiedNormalFlowBC.h"

registerMooseObject("NavierStokesApp", NSMomentumInviscidSpecifiedNormalFlowBC);

InputParameters
NSMomentumInviscidSpecifiedNormalFlowBC::validParams()
{
  InputParameters params = NSMomentumInviscidBC::validParams();
  params.addClassDescription("Momentum equation boundary condition in which pressure is specified "
                             "(given) and the value of the convective part is allowed to vary (is "
                             "computed implicitly).");
  params.addRequiredCoupledVar(NS::pressure, "pressure");
  params.addRequiredParam<Real>(
      "rhou_udotn", "The _component'th entry of the (rho*u)(u.n) vector for this boundary");
  return params;
}

NSMomentumInviscidSpecifiedNormalFlowBC::NSMomentumInviscidSpecifiedNormalFlowBC(
    const InputParameters & parameters)
  : NSMomentumInviscidBC(parameters),
    _pressure(coupledValue(NS::pressure)),
    _rhou_udotn(getParam<Real>("rhou_udotn"))
{
}

Real
NSMomentumInviscidSpecifiedNormalFlowBC::computeQpResidual()
{
  return pressureQpResidualHelper(_pressure[_qp]) + convectiveQpResidualHelper(_rhou_udotn);
}

Real
NSMomentumInviscidSpecifiedNormalFlowBC::computeQpJacobian()
{
  // There is no Jacobian for the convective term when (rho*u)(u.n) is specified,
  // so all we have left is the pressure jacobian.  The on-diagonal variable number
  // is _component+1
  return pressureQpJacobianHelper(_component + 1);
}

Real
NSMomentumInviscidSpecifiedNormalFlowBC::computeQpOffDiagJacobian(unsigned jvar)
{
  if (isNSVariable(jvar))
    return pressureQpJacobianHelper(mapVarNumber(jvar));
  else
    return 0.0;
}

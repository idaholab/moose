//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSGravityPower.h"

registerMooseObject("NavierStokesApp", NSGravityPower);

InputParameters
NSGravityPower::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("This class computes the momentum contributed by gravity.");
  params.addRequiredCoupledVar("momentum", "");
  params.addRequiredParam<Real>("acceleration", "The body force vector component.");
  return params;
}

NSGravityPower::NSGravityPower(const InputParameters & parameters)
  : Kernel(parameters),
    _momentum_var(coupled("momentum")),
    _momentum(coupledValue("momentum")),
    _acceleration(getParam<Real>("acceleration"))
{
}

Real
NSGravityPower::computeQpResidual()
{
  // -(rho * U * g) * phi
  return -_momentum[_qp] * _acceleration * _test[_i][_qp];
}

Real
NSGravityPower::computeQpJacobian()
{
  return 0.0;
}

Real
NSGravityPower::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _momentum_var)
    return -_phi[_j][_qp] * _acceleration * _test[_i][_qp];

  return 0.0;
}

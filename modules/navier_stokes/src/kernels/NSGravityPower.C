/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "NSGravityPower.h"

template <>
InputParameters
validParams<NSGravityPower>()
{
  InputParameters params = validParams<Kernel>();
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

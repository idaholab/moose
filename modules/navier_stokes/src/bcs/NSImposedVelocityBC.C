/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "NSImposedVelocityBC.h"

template<>
InputParameters validParams<NSImposedVelocityBC>()
{
  InputParameters params = validParams<NodalBC>();
  params.addRequiredCoupledVar("rho", "");
  params.addRequiredParam<Real>("desired_velocity", "");
  return params;
}

NSImposedVelocityBC::NSImposedVelocityBC(const InputParameters & parameters) :
    NodalBC(parameters),
    _rho(coupledValue("rho")),
    _desired_velocity(getParam<Real>("desired_velocity"))
{
}

Real NSImposedVelocityBC::computeQpResidual()
{
  // Return the difference between the current momentum and the desired value
  // (rho*u) - rho*desired_velocity
  return _u[_qp] - (_rho[_qp] * _desired_velocity);
}

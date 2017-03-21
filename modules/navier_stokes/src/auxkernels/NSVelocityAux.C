/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

// Navier-Stokes includes
#include "NSVelocityAux.h"
#include "NS.h"

template <>
InputParameters
validParams<NSVelocityAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addClassDescription("Velocity auxiliary value.");
  params.addRequiredCoupledVar(NS::density, "Density (conserved form)");
  params.addRequiredCoupledVar("momentum", "Momentum (conserved form)");
  return params;
}

NSVelocityAux::NSVelocityAux(const InputParameters & parameters)
  : AuxKernel(parameters), _rho(coupledValue(NS::density)), _momentum(coupledValue("momentum"))
{
}

Real
NSVelocityAux::computeValue()
{
  return _momentum[_qp] / _rho[_qp];
}

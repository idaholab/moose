/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "NSMachAux.h"
#include "MooseMesh.h"

template<>
InputParameters validParams<NSMachAux>()
{
  InputParameters params = validParams<AuxKernel>();

  params.addRequiredCoupledVar("u", "x-velocity");
  params.addCoupledVar("v", "y-velocity"); // Only required in >= 2D
  params.addCoupledVar("w", "z-velocity"); // Only required in 3D...
  params.addRequiredCoupledVar("temperature", "");
  params.addRequiredParam<Real>("gamma", "Ratio of specific heats");
  params.addRequiredParam<Real>("R", "Gas constant.");

  return params;
}

NSMachAux::NSMachAux(const InputParameters & parameters) :
    AuxKernel(parameters),
    _u_vel(coupledValue("u")),
    _v_vel(_mesh.dimension() >= 2 ? coupledValue("v") : _zero),
    _w_vel(_mesh.dimension() == 3 ? coupledValue("w") : _zero),
    _temperature(coupledValue("temperature")),
    _gamma(getParam<Real>("gamma")),
    _R(getParam<Real>("R"))
{
}

Real
NSMachAux::computeValue()
{
  const RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  // If temperature is negative, the solution is probably not going to
  // be very good anyway... in that case we will just compute the Mach
  // number based on |T| and avoid NaNs.  We don't check whether
  // temperature is somehow exactly 0.0, though.
  return vel.norm() / std::sqrt(_gamma * _R * std::abs(_temperature[_qp]));
}

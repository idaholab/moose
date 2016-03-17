/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "NSTemperatureAux.h"
#include "MooseMesh.h"

template<>
InputParameters validParams<NSTemperatureAux>()
{
  InputParameters params = validParams<AuxKernel>();

  // Mark variables as required
  params.addRequiredCoupledVar("rho", "");
  params.addRequiredCoupledVar("u", "x-velocity");
  params.addRequiredCoupledVar("v", "y-velocity");
  params.addCoupledVar("w", "z-velocity"); // Only required in 3D...
  params.addRequiredCoupledVar("rhoe", "");

  // Parameters with default values
  params.addRequiredParam<Real>("R", "Gas constant.");
  params.addRequiredParam<Real>("gamma", "Ratio of specific heats.");

  return params;
}

NSTemperatureAux::NSTemperatureAux(const InputParameters & parameters) :
    AuxKernel(parameters),
    _rho(coupledValue("rho")),
    _u_vel(coupledValue("u")),
    _v_vel(coupledValue("v")),
    _w_vel(_mesh.dimension() == 3 ? coupledValue("w") : _zero),
    _rhoe(coupledValue("rhoe")),
    _R(getParam<Real>("R")),
    _gamma(getParam<Real>("gamma"))
{
}

Real
NSTemperatureAux::computeValue()
{
  Real V2 =
    _u_vel[_qp]*_u_vel[_qp] +
    _v_vel[_qp]*_v_vel[_qp] +
    _w_vel[_qp]*_w_vel[_qp];

  // Internal Energy = Total Energy - Kinetic
  Real e_i = (_rhoe[_qp] / _rho[_qp]) - 0.5 * V2;

  // T = e_i / cv
  Real cv = _R / (_gamma-1.);
  return e_i / cv;
}

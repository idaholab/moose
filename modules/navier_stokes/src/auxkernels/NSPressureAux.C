/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "NSPressureAux.h"
#include "MooseMesh.h"

template<>
InputParameters validParams<NSPressureAux>()
{
  InputParameters params = validParams<AuxKernel>();

  // Mark variables as required
  params.addRequiredCoupledVar("rho", "");
  params.addRequiredCoupledVar("u", "x-velocity");
  params.addRequiredCoupledVar("v", "y-velocity");
  params.addCoupledVar("w", "z-velocity"); // Only required in 3D...
  params.addRequiredCoupledVar("rhoe", "");

  // Parameters with default values
  params.addRequiredParam<Real>("gamma", "Ratio of specific heats");

  return params;
}

NSPressureAux::NSPressureAux(const InputParameters & parameters) :
    AuxKernel(parameters),
    _rho(coupledValue("rho")),
    _u_vel(coupledValue("u")),
    _v_vel(coupledValue("v")),
    _w_vel(_mesh.dimension() == 3 ? coupledValue("w") : _zero),
    _rhoe(coupledValue("rhoe")),
    _gamma(getParam<Real>("gamma")) // can't use Material properties in Nodal Aux...
{
}

Real
NSPressureAux::computeValue()
{
  // Velocity vector, squared
  const Real V2 = _u_vel[_qp]*_u_vel[_qp] + _v_vel[_qp]*_v_vel[_qp] + _w_vel[_qp]*_w_vel[_qp];

  // P = (gam-1) * ( rho*e_t - 1/2 * rho * V^2)
  return (_gamma - 1)*(_rhoe[_qp] - 0.5 * _rho[_qp] * V2);
}

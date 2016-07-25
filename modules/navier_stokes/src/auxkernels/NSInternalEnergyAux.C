/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "NSInternalEnergyAux.h"
#include "MooseMesh.h"

template<>
InputParameters validParams<NSInternalEnergyAux>()
{
  InputParameters params = validParams<AuxKernel>();

  params.addRequiredCoupledVar("rho", "density");
  params.addRequiredCoupledVar("u", "x-velocity");
  params.addCoupledVar("v", "y-velocity"); // Only required in >= 2D
  params.addCoupledVar("w", "z-velocity"); // Only required in 3D...
  params.addRequiredCoupledVar("rhoE", "total energy");

  return params;
}

NSInternalEnergyAux::NSInternalEnergyAux(const InputParameters & parameters) :
    AuxKernel(parameters),
    _rho(coupledValue("rho")),
    _u_vel(coupledValue("u")),
    _v_vel(_mesh.dimension() >= 2 ? coupledValue("v") : _zero),
    _w_vel(_mesh.dimension() == 3 ? coupledValue("w") : _zero),
    _rhoE(coupledValue("rhoE"))
{
}

Real
NSInternalEnergyAux::computeValue()
{
  const RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  return _rhoE[_qp]/_rho[_qp] - 0.5 * vel.norm_sq();
}

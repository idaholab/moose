/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

// Navier-Stokes includes
#include "NSInternalEnergyAux.h"
#include "NS.h"

// MOOSE includes
#include "MooseMesh.h"

template <>
InputParameters
validParams<NSInternalEnergyAux>()
{
  InputParameters params = validParams<AuxKernel>();

  params.addClassDescription("Auxiliary kernel for computing the internal energy of the fluid.");
  params.addRequiredCoupledVar(NS::density, "density");
  params.addRequiredCoupledVar(NS::velocity_x, "x-velocity");
  params.addCoupledVar(NS::velocity_y, "y-velocity"); // Only required in >= 2D
  params.addCoupledVar(NS::velocity_z, "z-velocity"); // Only required in 3D...
  params.addRequiredCoupledVar(NS::total_energy, "total energy");

  return params;
}

NSInternalEnergyAux::NSInternalEnergyAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _rho(coupledValue(NS::density)),
    _u_vel(coupledValue(NS::velocity_x)),
    _v_vel(_mesh.dimension() >= 2 ? coupledValue(NS::velocity_y) : _zero),
    _w_vel(_mesh.dimension() == 3 ? coupledValue(NS::velocity_z) : _zero),
    _rhoE(coupledValue(NS::total_energy))
{
}

Real
NSInternalEnergyAux::computeValue()
{
  const RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  return _rhoE[_qp] / _rho[_qp] - 0.5 * vel.norm_sq();
}

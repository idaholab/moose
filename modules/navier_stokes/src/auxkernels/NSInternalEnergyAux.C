//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes includes
#include "NSInternalEnergyAux.h"
#include "NS.h"

// MOOSE includes
#include "MooseMesh.h"

registerMooseObject("NavierStokesApp", NSInternalEnergyAux);

InputParameters
NSInternalEnergyAux::validParams()
{
  InputParameters params = AuxKernel::validParams();

  params.addClassDescription("Auxiliary kernel for computing the internal energy of the fluid.");
  params.addRequiredCoupledVar(NS::density, "density");
  params.addRequiredCoupledVar(NS::velocity_x, "x-velocity");
  params.addCoupledVar(NS::velocity_y, "y-velocity"); // Only required in >= 2D
  params.addCoupledVar(NS::velocity_z, "z-velocity"); // Only required in 3D...
  params.addRequiredCoupledVar(NS::total_energy_density, "total energy");

  return params;
}

NSInternalEnergyAux::NSInternalEnergyAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _rho(coupledValue(NS::density)),
    _u_vel(coupledValue(NS::velocity_x)),
    _v_vel(_mesh.dimension() >= 2 ? coupledValue(NS::velocity_y) : _zero),
    _w_vel(_mesh.dimension() == 3 ? coupledValue(NS::velocity_z) : _zero),
    _rho_et(coupledValue(NS::total_energy_density))
{
}

Real
NSInternalEnergyAux::computeValue()
{
  const RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  return _rho_et[_qp] / _rho[_qp] - 0.5 * vel.norm_sq();
}

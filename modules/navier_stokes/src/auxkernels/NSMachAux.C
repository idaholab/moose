//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes includes
#include "NSMachAux.h"
#include "NS.h"

// FluidProperties includes
#include "SinglePhaseFluidProperties.h"

// MOOSE includes
#include "MooseMesh.h"

registerMooseObject("NavierStokesApp", NSMachAux);

InputParameters
NSMachAux::validParams()
{
  InputParameters params = AuxKernel::validParams();

  params.addClassDescription(
      "Auxiliary kernel for computing the Mach number assuming an ideal gas.");
  params.addRequiredCoupledVar(NS::velocity_x, "x-velocity");
  params.addCoupledVar(NS::velocity_y, "y-velocity"); // Only required in >= 2D
  params.addCoupledVar(NS::velocity_z, "z-velocity"); // Only required in 3D...
  params.addRequiredCoupledVar(NS::specific_volume, "specific volume");
  params.addCoupledVar(NS::specific_internal_energy, "internal energy");
  params.addDeprecatedCoupledVar(
      NS::internal_energy, NS::specific_internal_energy, "January 1, 2022");
  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "The name of the user object for fluid properties");

  return params;
}

NSMachAux::NSMachAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _u_vel(coupledValue(NS::velocity_x)),
    _v_vel(_mesh.dimension() >= 2 ? coupledValue(NS::velocity_y) : _zero),
    _w_vel(_mesh.dimension() == 3 ? coupledValue(NS::velocity_z) : _zero),
    _specific_volume(coupledValue(NS::specific_volume)),
    _specific_internal_energy(coupledValue(NS::specific_internal_energy)),
    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties"))
{
}

Real
NSMachAux::computeValue()
{
  return RealVectorValue(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]).norm() /
         _fp.c_from_v_e(_specific_volume[_qp], _specific_internal_energy[_qp]);
}

/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

// Navier-Stokes includes
#include "NSMachAux.h"
#include "NS.h"

// FluidProperties includes
#include "IdealGasFluidProperties.h"

// MOOSE includes
#include "MooseMesh.h"

template <>
InputParameters
validParams<NSMachAux>()
{
  InputParameters params = validParams<AuxKernel>();

  params.addClassDescription(
      "Auxiliary kernel for computing the Mach number assuming an ideal gas.");
  params.addRequiredCoupledVar(NS::velocity_x, "x-velocity");
  params.addCoupledVar(NS::velocity_y, "y-velocity"); // Only required in >= 2D
  params.addCoupledVar(NS::velocity_z, "z-velocity"); // Only required in 3D...
  params.addRequiredCoupledVar(NS::specific_volume, "specific volume");
  params.addRequiredCoupledVar(NS::internal_energy, "internal energy");
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
    _internal_energy(coupledValue(NS::internal_energy)),
    _fp(getUserObject<IdealGasFluidProperties>("fluid_properties"))
{
}

Real
NSMachAux::computeValue()
{
  return RealVectorValue(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]).norm() /
         _fp.c(_specific_volume[_qp], _internal_energy[_qp]);
}

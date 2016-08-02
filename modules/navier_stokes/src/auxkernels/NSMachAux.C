/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "NSMachAux.h"

// FluidProperties includes
#include "IdealGasFluidProperties.h"

// MOOSE includes
#include "MooseMesh.h"

template<>
InputParameters validParams<NSMachAux>()
{
  InputParameters params = validParams<AuxKernel>();

  params.addRequiredCoupledVar("u", "x-velocity");
  params.addCoupledVar("v", "y-velocity"); // Only required in >= 2D
  params.addCoupledVar("w", "z-velocity"); // Only required in 3D...
  params.addRequiredCoupledVar("specific_volume", "");
  params.addRequiredCoupledVar("internal_energy", "");
  params.addRequiredParam<UserObjectName>("fluid_properties", "The name of the user object for fluid properties");

  return params;
}

NSMachAux::NSMachAux(const InputParameters & parameters) :
    AuxKernel(parameters),
    _u_vel(coupledValue("u")),
    _v_vel(_mesh.dimension() >= 2 ? coupledValue("v") : _zero),
    _w_vel(_mesh.dimension() == 3 ? coupledValue("w") : _zero),
    _specific_volume(coupledValue("specific_volume")),
    _internal_energy(coupledValue("internal_energy")),
    _fp(getUserObject<IdealGasFluidProperties>("fluid_properties"))
{
}

Real
NSMachAux::computeValue()
{
  return RealVectorValue(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]).norm() / _fp.c(_specific_volume[_qp], _internal_energy[_qp]);
}

/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

// Navier-Stokes inclues
#include "NSPressureAux.h"
#include "NS.h"

// FluidProperties includes
#include "IdealGasFluidProperties.h"

// MOOSE includes
#include "MooseMesh.h"

template <>
InputParameters
validParams<NSPressureAux>()
{
  InputParameters params = validParams<AuxKernel>();

  params.addClassDescription("Nodal auxiliary variable, for computing pressure at the nodes.");
  // Mark variables as required
  params.addRequiredCoupledVar(NS::specific_volume, "specific volume");
  params.addRequiredCoupledVar(NS::internal_energy, "internal energy");
  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "The name of the user object for fluid properties");

  return params;
}

NSPressureAux::NSPressureAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _specific_volume(coupledValue(NS::specific_volume)),
    _internal_energy(coupledValue(NS::internal_energy)),
    _fp(getUserObject<IdealGasFluidProperties>("fluid_properties"))
{
}

Real
NSPressureAux::computeValue()
{
  return _fp.pressure(_specific_volume[_qp], _internal_energy[_qp]);
}

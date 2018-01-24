/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "NSStagnationBC.h"

// FluidProperties includes
#include "IdealGasFluidProperties.h"

// MOOSE includes
#include "MooseMesh.h"

template <>
InputParameters
validParams<NSStagnationBC>()
{
  InputParameters params = validParams<NodalBC>();
  params.addClassDescription(
      "This is the base class for the 'imposed stagnation' value boundary conditions.");
  params.addRequiredCoupledVar("mach", "Mach number");
  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "The name of the user object for fluid properties");
  return params;
}

NSStagnationBC::NSStagnationBC(const InputParameters & parameters)
  : NodalBC(parameters),
    _mach(coupledValue("mach")),
    _fp(getUserObject<IdealGasFluidProperties>("fluid_properties"))
{
}

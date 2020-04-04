//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSStagnationBC.h"

// FluidProperties includes
#include "IdealGasFluidProperties.h"

// MOOSE includes
#include "MooseMesh.h"

InputParameters
NSStagnationBC::validParams()
{
  InputParameters params = NodalBC::validParams();
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

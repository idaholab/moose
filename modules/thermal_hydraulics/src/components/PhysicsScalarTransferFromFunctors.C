//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhysicsScalarTransferFromFunctors.h"

registerMooseObject("ThermalHydraulicsApp", PhysicsScalarTransferFromFunctors);

InputParameters
PhysicsScalarTransferFromFunctors::validParams()
{
  InputParameters params = PhysicsScalarTransferBase::validParams();
  params.addRequiredParam<std::vector<MooseFunctorName>>(
      "wall_scalar_values", "Vector of functors providing the scalar variable values on the wall");

  params.addClassDescription(
      "Scalar transfer specified by a wall temperature into a channel using Physics.");
  return params;
}

PhysicsScalarTransferFromFunctors::PhysicsScalarTransferFromFunctors(
    const InputParameters & parameters)
  : PhysicsScalarTransferBase(parameters),
    _wall_scalar_functors(getParam<std::vector<MooseFunctorName>>("wall_scalar_values"))
{
}

void
PhysicsScalarTransferFromFunctors::init()
{
  PhysicsScalarTransferBase::init();
  for (auto th_phys : _th_physics)
    th_phys->addWallScalarFlux(name(), ThermalHydraulicsFlowPhysics::FixedScalarValue);
}

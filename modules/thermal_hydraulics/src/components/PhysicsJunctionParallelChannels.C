//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhysicsJunctionParallelChannels.h"

registerMooseObjectAliased("ThermalHydraulicsApp",
                           PhysicsJunctionParallelChannels,
                           "JunctionParallelChannels");

InputParameters
PhysicsJunctionParallelChannels::validParams()
{
  InputParameters params = PhysicsVolumeJunction::validParams();

  params.addClassDescription("Junction between two or more parallel flow channels.");

  return params;
}

PhysicsJunctionParallelChannels::PhysicsJunctionParallelChannels(const InputParameters & params)
  : PhysicsVolumeJunction(params)
{
}

void
PhysicsJunctionParallelChannels::init()
{
  PhysicsFlowJunction::init();

  // For now, we do this here. We could consider doing it elsewhere
  for (auto th_phys : _th_physics)
    th_phys->setJunction(name(), ThermalHydraulicsFlowPhysics::ParallelChannels);
}

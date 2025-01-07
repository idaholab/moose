//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhysicsSolidWall.h"

registerMooseObjectAliased("ThermalHydraulicsApp", PhysicsSolidWall, "SolidWall");

InputParameters
PhysicsSolidWall::validParams()
{
  InputParameters params = PhysicsFlowBoundary::validParams();
  params.addClassDescription("Adds the boundary condition for a wall in single phase flow");
  return params;
}

PhysicsSolidWall::PhysicsSolidWall(const InputParameters & params) : PhysicsFlowBoundary(params) {}

void
PhysicsSolidWall::init()
{
  PhysicsFlowBoundary::init();
  // For now, we do this here. We could consider doing it elsewhere
  for (auto th_phys : _th_physics)
    th_phys->setOutlet(name(), ThermalHydraulicsFlowPhysics::SolidWall);
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhysicsFreeBoundary.h"
#include "FlowModelSinglePhase.h"

registerMooseObject("ThermalHydraulicsApp", PhysicsFreeBoundary);

InputParameters
PhysicsFreeBoundary::validParams()
{
  InputParameters params = FlowBoundary1Phase::validParams();
  params.addClassDescription(
      "Component to create a free flow boundary for flow using thermal hydraulic Physics.");
  return params;
}

PhysicsFreeBoundary::PhysicsFreeBoundary(const InputParameters & parameters)
  : PhysicsFlowBoundary(parameters)
{
}

void
PhysicsFreeBoundary::addMooseObjects()
{
  // We set this as an outlet as NSFV supports these as outlets
  for (auto th_phys : _th_physics)
    th_phys->setOutlet(name(), ThermalHydraulicsFlowPhysics::FreeBoundary);
}

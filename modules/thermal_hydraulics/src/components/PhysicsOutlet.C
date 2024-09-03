//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhysicsOutlet.h"
#include "ThermalHydraulicsFlowPhysics.h"

registerMooseObject("ThermalHydraulicsApp", PhysicsOutlet);

InputParameters
PhysicsOutlet::validParams()
{
  InputParameters params = PhysicsFlowBoundary::validParams();
  params.addRequiredParam<Real>("p", "Prescribed pressure [Pa]");
  params.declareControllable("p");
  params.addClassDescription(
      "Boundary condition with prescribed pressure for flow channels using Physics.");
  return params;
}

PhysicsOutlet::PhysicsOutlet(const InputParameters & params) : PhysicsFlowBoundary(params) {}

void
PhysicsOutlet::check() const
{
  PhysicsFlowBoundary::check();
}

void
PhysicsOutlet::addMooseObjects()
{
  // For now, we do this here. We could consider doing it elsewhere
  for (auto th_phys : _th_physics)
    th_phys->setOutlet(name(), ThermalHydraulicsFlowPhysics::FixedPressure);
}

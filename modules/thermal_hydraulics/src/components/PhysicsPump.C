//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhysicsPump.h"

registerMooseObjectAliased("ThermalHydraulicsApp", PhysicsPump, "Pump");

InputParameters
PhysicsPump::validParams()
{
  InputParameters params = PhysicsVolumeJunction::validParams();

  params.addRequiredParam<Real>("head", "Pump head [m]");
  params.makeParamRequired<Real>("A_ref");
  params.declareControllable("head");

  params.addClassDescription("Pump between two flow channels that has a non-zero volume");

  return params;
}

PhysicsPump::PhysicsPump(const InputParameters & params)
  : PhysicsVolumeJunction(params), _head(getParam<Real>("head"))
{
}

void
PhysicsPump::init()
{
  PhysicsFlowJunction::init();

  // For now, we do this here. We could consider doing it elsewhere
  for (auto th_phys : _th_physics)
    th_phys->setJunction(name(), ThermalHydraulicsFlowPhysics::Pump);
}

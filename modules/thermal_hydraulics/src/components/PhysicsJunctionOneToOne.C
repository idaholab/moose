//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhysicsJunctionOneToOne.h"
#include "PhysicsFlowChannel.h"
#include "THMMesh.h"

registerMooseObject("ThermalHydraulicsApp", PhysicsJunctionOneToOne);

InputParameters
PhysicsJunctionOneToOne::validParams()
{
  InputParameters params = PhysicsFlowJunction::validParams();

  params.addClassDescription("Junction connecting one flow channel to one other flow channel, both "
                             "leveraging Physics to define the flow discretization");

  return params;
}

PhysicsJunctionOneToOne::PhysicsJunctionOneToOne(const InputParameters & params)
  : PhysicsFlowJunction(params)
{
}

void
PhysicsJunctionOneToOne::setupMesh()
{
  PhysicsFlowJunction::setupMesh();

  if (_connected_elems.size() == 2)
    getTHMProblem().augmentSparsity(_connected_elems[0], _connected_elems[1]);
}

void
PhysicsJunctionOneToOne::init()
{
  PhysicsFlowJunction::init();
}

void
PhysicsJunctionOneToOne::check() const
{
  PhysicsFlowJunction::check();

  // Check that there are exactly 2 connections
  checkNumberOfConnections(2);
}

void
PhysicsJunctionOneToOne::addMooseObjects()
{
  // For now, we do this here. We could consider doing it elsewhere
  for (auto th_phys : _th_physics)
    th_phys->setJunction(name(), ThermalHydraulicsFlowPhysics::OneToOne);
}

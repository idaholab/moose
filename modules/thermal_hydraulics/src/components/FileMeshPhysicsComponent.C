//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FileMeshPhysicsComponent.h"
#include "PhysicsBase.h"

registerMooseObject("ThermalHydraulicsApp", FileMeshPhysicsComponent);

InputParameters
FileMeshPhysicsComponent::validParams()
{
  InputParameters params = FileMeshComponent::validParams();

  params.addClassDescription(
      "Component using a mesh from a file with one or more Physics active on it.");
  params.addParam<std::vector<PhysicsName>>("physics", "Physics object(s) active on the Component");
  return params;
}

FileMeshPhysicsComponent::FileMeshPhysicsComponent(const InputParameters & parameters)
  : FileMeshComponent(parameters)
{
}

void
FileMeshPhysicsComponent::addRelationshipManagers(Moose::RelationshipManagerType /*in_rm_type*/)
{
  // TODO: We ll just add late relationship managers
  // At this point in the setup, we do not have a problem, so we cannot retrieve a Physics. We can
  // send the default ghosting for the physics, but that's it.
  addRelationshipManagersFromParameters(PhysicsBase::validParams());
}

void
FileMeshPhysicsComponent::init()
{
  FileMeshComponent::init();

  // Before this point, we did not have a problem, so we could not retrieve the physics
  for (const auto & physics_name : getParam<std::vector<PhysicsName>>("physics"))
    _physics.push_back(_app.actionWarehouse().getPhysics<PhysicsBase>(physics_name));

  for (auto physics : _physics)
    physics->addBlocks(getSubdomainNames());
}

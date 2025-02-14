//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComponentPhysicsInterface.h"

InputParameters
ComponentPhysicsInterface::validParams()
{
  auto params = ActionComponent::validParams();
  params.addParam<std::vector<PhysicsName>>(
      "physics", {}, "Physics object(s) active on the Component");
  return params;
}

ComponentPhysicsInterface::ComponentPhysicsInterface(const InputParameters & params)
  : ActionComponent(params), _physics_names(getParam<std::vector<PhysicsName>>("physics"))
{
  // Adding ActionComponents must be done after adding Physics
  for (const auto & physics_name : getParam<std::vector<PhysicsName>>("physics"))
    _physics.push_back(getMooseApp().actionWarehouse().getPhysics<PhysicsBase>(physics_name));

  addRequiredTask("init_component_physics");
}

void
ComponentPhysicsInterface::addPhysics()
{
  for (auto physics : _physics)
  {
    if (_verbose)
      mooseInfoRepeated("Adding Physics '" + physics->name() + "' on component '" + name() +
                        "' on blocks '" + Moose::stringify(_blocks) + "'");
    physics->addComponent(*this);
  }
}

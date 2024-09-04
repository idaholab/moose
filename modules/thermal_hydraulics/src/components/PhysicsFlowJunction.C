//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhysicsFlowJunction.h"
#include "PhysicsFlowChannel.h"

InputParameters
PhysicsFlowJunction::validParams()
{
  InputParameters params = FlowJunction::validParams();
  return params;
}

PhysicsFlowJunction::PhysicsFlowJunction(const InputParameters & params) : FlowJunction(params) {}

void
PhysicsFlowJunction::init()
{
  FlowJunction::init();

  for (const auto & connection : _connections)
  {
    const std::string comp_name = connection._component_name;
    if (hasComponentByName<PhysicsFlowChannel>(comp_name))
    {
      const PhysicsFlowChannel & comp =
          getTHMProblem().getComponentByName<PhysicsFlowChannel>(comp_name);
      for (const auto physics : comp.getPhysics())
        _th_physics.insert(physics);
    }
  }
}

void
PhysicsFlowJunction::check() const
{
  FlowJunction::check();

  for (const auto & comp_name : _connected_component_names)
    checkComponentOfTypeExistsByName<PhysicsFlowChannel>(comp_name);
}

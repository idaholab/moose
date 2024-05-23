//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhysicsComponentHelper.h"

InputParameters
PhysicsComponentHelper::validParams()
{
  auto params = emptyInputParameters();
  params.addParam<std::vector<PhysicsName>>(
      "physics", {}, "Physics object(s) active on the Component");
  return params;
}

PhysicsComponentHelper::PhysicsComponentHelper(const InputParameters & params)
  : ComponentAction(params), _physics_names(getParam<std::vector<PhysicsName>>("physics"))
{
  // Should be done later?
  for (const auto & physics_name : getParam<std::vector<PhysicsName>>("physics"))
    _physics.push_back(getMooseApp().actionWarehouse().getPhysics<PhysicsBase>(physics_name));
}

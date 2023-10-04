//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddPhysicsDiscretizationAction.h"
#include "FEProblem.h"
#include "PhysicsBase.h"

registerMooseAction("MooseApp", AddPhysicsDiscretizationAction, "add_physics_discretization");

InputParameters
AddPhysicsDiscretizationAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a Discretization object (nested in Physics) to the simulation.");
  return params;
}

AddPhysicsDiscretizationAction::AddPhysicsDiscretizationAction(const InputParameters & params)
  : MooseObjectAction(params)
{
}

void
AddPhysicsDiscretizationAction::act()
{
  if (_current_task == "add_physics_discretization")
  {
    // Parse the physics name from the action name (parse out Physics/ & /Discretization)
    auto physics_name = _awh.getCurrentActionName();
    physics_name.replace(physics_name.size() - 15, 15, "");
    physics_name.replace(0, 8, "");

    // Add type from detected type
    // TODO: This should not be necessary
    _moose_object_pars.set<std::string>("_type") = getParam<std::string>("type");
    _moose_object_pars.set<std::string>("_object_name") = "discretization_" + physics_name;
    _moose_object_pars.set<bool>("enable") = true;

    auto physics = _problem->getPhysics(physics_name);
    physics->addDiscretization(_moose_object_pars);
    physics->createDiscretizedPhysics();
  }
}

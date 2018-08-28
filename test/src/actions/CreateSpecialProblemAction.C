//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CreateSpecialProblemAction.h"
#include "MooseApp.h"

registerMooseAction("MooseTestApp", CreateSpecialProblemAction, "meta_action");

template <>
InputParameters
validParams<CreateSpecialProblemAction>()
{
  InputParameters params = validParams<Action>();
  params.addParam<std::string>("name", "Test Problem", "The name of the Problem object");
  return params;
}

CreateSpecialProblemAction::CreateSpecialProblemAction(InputParameters parameters)
  : Action(parameters)
{
}

void
CreateSpecialProblemAction::act()
{
  auto _action_params = _action_factory.getValidParams("CreateProblemAction");

  // Set a specific type here, could be determined from other parameters
  _action_params.set<std::string>("type") = "MooseTestProblem";

  // Create the Action that will be called to create the problem!
  auto action =
      _action_factory.create("CreateProblemAction", getParam<std::string>("name"), _action_params);

  _awh.addActionBlock(action);
}

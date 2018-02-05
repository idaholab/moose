//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MetaMultipleTasksAction.h"
#include "ActionFactory.h"
#include "ActionWarehouse.h"

template <>
InputParameters
validParams<MetaMultipleTasksAction>()
{
  InputParameters params = validParams<Action>();
  return params;
}

MetaMultipleTasksAction::MetaMultipleTasksAction(const InputParameters & params) : Action(params) {}

void
MetaMultipleTasksAction::act()
{
  InputParameters object_params = _action_factory.getValidParams("AddMatAndKernel");
  object_params.set<ActionWarehouse *>("awh") = &_awh;
  object_params.set<bool>("verbose") = true;

  // Create and Add First Variable Action
  auto action = _action_factory.create("AddMatAndKernel", "AddMatAndKernel", object_params);
  _awh.addActionBlock(action);
}

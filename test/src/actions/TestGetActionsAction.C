//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestGetActionsAction.h"

#include "ActionWarehouse.h"
#include "AddMaterialAction.h"
#include "MooseApp.h"

#include <libmesh/parallel_implementation.h>

template <>
InputParameters
validParams<TestGetActionsAction>()
{
  InputParameters params = validParams<Action>();
  params.addClassDescription(
      "An action demonstrating how an action can interact with other actions");
  return params;
}

TestGetActionsAction::TestGetActionsAction(const InputParameters & params) : Action(params) {}

void
TestGetActionsAction::act()
{
  // use AddMaterialAction as an example
  auto actions = _awh.getActions<AddMaterialAction>();

  // test on each processor
  for (auto i = beginIndex(actions); i < actions.size(); ++i)
  {
    if (i > 0)
      if (actions[i]->name() < actions[i - 1]->name())
        mooseError("actions are not sorted properly");
    auto action = &_awh.getAction<AddMaterialAction>(actions[i]->name());
    if (actions[i] != action)
      mooseError("getAction is returning the wrong action, something is really wrong");
  }

  // test to make sure all actions are consistent across processors
  auto & comm = _app.comm();
  if (comm.rank() == 0)
  {
    for (unsigned int pid = 1; pid < comm.size(); ++pid)
    {
      auto size = actions.size();
      comm.receive(pid, size);
      if (size != actions.size())
        mooseError("error occurs during getting actions, sizes of actions on master and rank ",
                   pid,
                   " are different");
      for (auto i = beginIndex(actions); i < actions.size(); ++i)
      {
        std::string action_name;
        comm.receive(pid, action_name);
        if (action_name != actions[i]->name())
          mooseError("error occurs during getting actions, action names are inconsistent on master "
                     "and rank ",
                     pid);
      }
    }
  }
  else
  {
    auto size = actions.size();
    comm.send(0, size);
    for (auto i = beginIndex(actions); i < actions.size(); ++i)
    {
      auto name = actions[i]->name();
      comm.send(0, name);
    }
  }

  _console << "Getting actions is successful" << std::endl;
}

//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "gtest_include.h"
#include "ActionUnitTest.h"

class MooseApp;
class ActionFactory;
class Factory;
class InputParameters;

/**
 * Unit test for the creation of wall-time checkpoints
 */
class AutoCheckpointTest : public ActionUnitTest
{
public:
  AutoCheckpointTest() : ActionUnitTest("MooseUnitApp") {}

protected:
  void buildActions()
  {
    // Add the auto-checkpoint action
    const auto action_type = "AutoCheckpointAction";
    InputParameters pars = _action_factory.getValidParams(action_type);
    pars.set<bool>("checkpoint") = false;
    auto action = _action_factory.create(action_type, "unit_test_auto_checkpoint", pars);
    _app->actionWarehouse().addActionBlock(action);
  }
  void runActions()
  {
    // Add the task: only needed if the task does not auto-register
    // Check the task
    mooseAssert(_action_factory.isRegisteredTask("auto_checkpoint_action"),
                "Should have registered the task");
    // Run the task
    _app->actionWarehouse().executeActionsWithAction("auto_checkpoint_action");
  }
};

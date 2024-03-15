//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AutoCheckpointAction.h"
#include "FEProblem.h"
#include "Checkpoint.h"
registerMooseAction("MooseApp", AutoCheckpointAction, "auto_checkpoint_action");

InputParameters
AutoCheckpointAction::validParams()
{
  InputParameters params = Action::validParams();

  return params;
}

AutoCheckpointAction::AutoCheckpointAction(const InputParameters & params) : Action(params) {}

void
AutoCheckpointAction::act()
{
  // if there's already a checkpoint object, we don't need to worry about creating a new
  // checkpoint
  const auto checkpoints = _app.getOutputWarehouse().getOutputs<Checkpoint>();
  const auto num_checkpoints = checkpoints.size();

  if (num_checkpoints > 1)
  {
    // Get most recently added Checkpoint object and error
    mooseError("Multiple checkpoints are not allowed. Check the input to ensure there "
               "is only one Checkpoint defined in the 'Outputs' block, including the "
               "shortcut syntax 'Outputs/checkpoint=true'.");
  }

  // We don't want to set up automatic checkpoints if we are not in the master app
  else if (_app.isUltimateMaster())
  {
    if (num_checkpoints == 0)
    {
      // If there isn't an existing checkpoint, init a new one
      auto cp_params = _factory.getValidParams("Checkpoint");
      cp_params.setParameters("checkpoint_type", CheckpointType::SYSTEM_CREATED);
      cp_params.set<bool>("_built_by_moose") = true;
      _problem->addOutput("Checkpoint", "checkpoint", cp_params);
    }

    else // num_checkpoints == 1
    {
      // Use the existing Checkpoint object, since we only need to/should make one object the
      // autosave
      checkpoints[0]->setAutosaveFlag(CheckpointType::USER_CREATED);
    }

    // Check for special half transient test harness case
    if (_app.testCheckpointHalfTransient())
    {
      // For half transient, we want to simulate a user-created checkpoint so
      // time_step_interval works correctly.
      const auto checkpoint = _app.getOutputWarehouse().getOutputs<Checkpoint>()[0];
      checkpoint->setAutosaveFlag(CheckpointType::USER_CREATED);
      checkpoint->_time_step_interval = 1;
    }
  }
}

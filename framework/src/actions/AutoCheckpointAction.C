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

  params.addClassDescription(
      "Action to create shortcut syntax-specified checkpoints and automatic checkpoints.");

  params.addParam<bool>("checkpoint", false, "Create checkpoint files using the default options.");
  params.addParam<bool>("disable_wall_time_checkpoints",
                        false,
                        "Disables the output of checkpoints based on elapsed wall time.");

  return params;
}

AutoCheckpointAction::AutoCheckpointAction(const InputParameters & params) : Action(params) {}

void
AutoCheckpointAction::act()
{
  // if there's already a checkpoint object, we don't need to worry about creating a new one
  const auto checkpoints = _app.getOutputWarehouse().getOutputs<Checkpoint>();
  const auto num_checkpoints = checkpoints.size();

  const bool shortcut_syntax = getParam<bool>("checkpoint");

  if (num_checkpoints > 1 || (num_checkpoints == 1 and shortcut_syntax))
    mooseError("Multiple checkpoints are not allowed. Check the input to ensure there "
               "is only one Checkpoint defined in the 'Outputs' block, including the "
               "shortcut syntax 'Outputs/checkpoint=true'.");

  if (num_checkpoints == 0)
  {
    // If there isn't an existing checkpoint, init a new one
    auto cp_params = _factory.getValidParams("Checkpoint");

    cp_params.set<bool>("_built_by_moose") = true;
    cp_params.set<bool>("disable_wall_time_checkpoints") =
        getParam<bool>("disable_wall_time_checkpoints");

    // We need to keep track of what type of checkpoint we are creating. system created means the
    // default value of 1 for time_step_interval is ignored.
    if (!shortcut_syntax)
      cp_params.set<CheckpointType>("checkpoint_type") = CheckpointType::SYSTEM_CREATED;

    // We only want checkpoints in subapps if the user requests them
    if (shortcut_syntax || _app.isUltimateMaster())
      _problem->addOutput("Checkpoint", "checkpoint", cp_params);
  }

  // Check for special half transient test harness case
  if (_app.testCheckpointHalfTransient() && _app.isUltimateMaster())
  {
    // For half transient, we want to simulate a user-created checkpoint so
    // time_step_interval works correctly.
    const auto checkpoint = _app.getOutputWarehouse().getOutputs<Checkpoint>()[0];
    checkpoint->setAutosaveFlag(CheckpointType::USER_CREATED);
    checkpoint->_time_step_interval = 1;
  }
}

//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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

  params.addParam<bool>("checkpoint", "Create checkpoint files using the default options.");

  return params;
}

AutoCheckpointAction::AutoCheckpointAction(const InputParameters & params) : Action(params) {}

void
AutoCheckpointAction::act()
{
  // if there's already a checkpoint object, we don't need to worry about creating a new one
  const auto checkpoints = _app.getOutputWarehouse().getOutputs<Checkpoint>();
  const auto num_checkpoints = checkpoints.size();

  const bool user_specified_checkpoint_behavior = isParamValid("checkpoint");
  const bool user_requested_checkpoint =
      user_specified_checkpoint_behavior && getParam<bool>("checkpoint");
  const bool user_requested_no_checkpoint =
      user_specified_checkpoint_behavior && (getParam<bool>("checkpoint") == false);

  if (num_checkpoints > 1)
    checkpoints[0]->mooseError("Multiple Checkpoint objects are not allowed and there is more than "
                               "one Checkpoint defined in the 'Outputs' block.");
  if (num_checkpoints == 1 && user_requested_checkpoint)
    paramError("checkpoint",
               "Shortcut checkpoint syntax cannot be used with another Checkpoint object in the "
               "'Outputs' block");

  if (num_checkpoints == 0 &&
      (user_requested_checkpoint || (_app.isUltimateMaster() && !user_requested_no_checkpoint)))
  {
    // If there isn't an existing checkpoint, init a new one
    auto cp_params = _factory.getValidParams("Checkpoint");

    cp_params.set<bool>("_built_by_moose") = true;
    if (!user_requested_checkpoint)
      // This is our auto-created wall-time based checkpoint, so disable the time step interval by
      // default
      cp_params.set<unsigned int>("time_step_interval", /*allow_override_by_common_output=*/true) =
          std::numeric_limits<unsigned int>::max();

    _problem->addOutput("Checkpoint", "checkpoint", cp_params);
  }

  // Check for special half transient test harness case
  if (_app.testCheckpointHalfTransient() && _app.isUltimateMaster())
  {
    // For half transient, we want to simulate a user-created checkpoint so
    // time_step_interval works correctly.
    const auto checkpoint = _app.getOutputWarehouse().getOutputs<Checkpoint>()[0];
    checkpoint->_time_step_interval = 1;
  }
}

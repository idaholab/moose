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
  if (_app.isUltimateMaster())
  {
    // if there's already a checkpoint object, we don't need to worry about creating a new
    // checkpoint
    if (!_app.getOutputWarehouse().getOutputs<Checkpoint>().empty())
    {

      // Take the first checkpoint object, since we only need to/should make one object the autosave
      _app.getOutputWarehouse().getOutputs<Checkpoint>()[0]->setAutosaveFlag(
          AutosaveType::MODIFIED_EXISTING);
      return;
    }

    // If there isn't an existing one, init a new one
    auto cp_params = _factory.getValidParams("Checkpoint");
    cp_params.setParameters("is_autosave", AutosaveType::SYSTEM_AUTOSAVE);
    cp_params.set<bool>("_built_by_moose") = true;
    _problem->addOutput("Checkpoint", "checkpoint", cp_params);
  }
}

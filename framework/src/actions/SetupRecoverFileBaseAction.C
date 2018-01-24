//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "SetupRecoverFileBaseAction.h"
#include "MooseApp.h"
#include "OutputWarehouse.h"
#include "Checkpoint.h"
#include "MooseObjectAction.h"

template <>
InputParameters
validParams<SetupRecoverFileBaseAction>()
{
  InputParameters params = validParams<Action>();
  return params;
}

SetupRecoverFileBaseAction::SetupRecoverFileBaseAction(InputParameters params) : Action(params) {}

void
SetupRecoverFileBaseAction::act()
{
  // Do nothing if the App is not recovering
  // Don't look for a checkpoint file unless we're the ultimate master app
  if (!_app.isRecovering() || !_app.isUltimateMaster())
    return;

  // Get the most current file, if it hasn't been set directly
  if (!_app.hasRecoverFileBase())
  {
    // Build the list of all possible checkpoint files for recover
    std::list<std::string> checkpoint_files = _app.getCheckpointFiles();

    // Grab the most recent one
    std::string recovery_file_base = MooseUtils::getLatestAppCheckpointFileBase(checkpoint_files);

    if (recovery_file_base.empty())
      mooseError("Unable to find suitable recovery file!");

    _app.setRecoverFileBase(recovery_file_base);
  }

  // Set the recover file base in the App
  _console << "\nUsing " << _app.getRecoverFileBase() << " for recovery.\n\n";
}

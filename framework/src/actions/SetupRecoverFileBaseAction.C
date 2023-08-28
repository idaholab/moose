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
#include "RestartableDataReader.h"

registerMooseAction("MooseApp", SetupRecoverFileBaseAction, "setup_recover_file_base");
registerMooseAction("MooseApp", SetupRecoverFileBaseAction, "recover_meta_data");

InputParameters
SetupRecoverFileBaseAction::validParams()
{
  InputParameters params = Action::validParams();
  return params;
}

SetupRecoverFileBaseAction::SetupRecoverFileBaseAction(const InputParameters & params)
  : Action(params)
{
}

void
SetupRecoverFileBaseAction::act()
{
  // Even during a normal run, we still need to check integrity of the data store to make
  // sure that all requested properties have been declared.
  if (_current_task == "recover_meta_data")
    _app.checkMetaDataIntegrity();

  // Do nothing if the App is not recovering
  // Don't look for a checkpoint file unless we're the ultimate master app
  if (!_app.isRecovering() || !_app.isUltimateMaster())
    return;

  if (_current_task == "setup_recover_file_base")
  {
    _app.setRestartRecoverFileBase(
        MooseUtils::convertLatestCheckpoint(_app.getRestartRecoverFileBase()));

    // Set the recover file base in the App
    mooseInfo("Using ", _app.getRestartRecoverFileBase(), " for recovery.");
  }
  else // recover_meta_data
  {
    // Make sure that all of the mesh meta-data attributes have been declared (after the mesh
    // generators have run.
    _app.loadRestartableMetaData(_app.getRestartRecoverFileBase() + _app.checkpointSuffix());
  }
}

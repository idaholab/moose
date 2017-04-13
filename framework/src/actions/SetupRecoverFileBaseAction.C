/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
    std::string recovery_file_base = MooseUtils::getRecoveryFileBase(checkpoint_files);

    if (recovery_file_base.empty())
      mooseError("Unable to find suitable recovery file!");

    _app.setRecoverFileBase(recovery_file_base);
  }

  // Set the recover file base in the App
  _console << "\nUsing " << _app.getRecoverFileBase() << " for recovery.\n\n";
}

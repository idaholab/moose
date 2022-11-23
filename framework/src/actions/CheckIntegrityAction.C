//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CheckIntegrityAction.h"
#include "ActionWarehouse.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", CheckIntegrityAction, "check_integrity");
registerMooseAction("MooseApp", CheckIntegrityAction, "check_integrity_early");

InputParameters
CheckIntegrityAction::validParams()
{
  InputParameters params = Action::validParams();
  return params;
}

CheckIntegrityAction::CheckIntegrityAction(const InputParameters & params) : Action(params) {}

void
CheckIntegrityAction::act()
{
  if (_current_task == "check_integrity_early")
  {
    if (!_app.getExecutioner() && !_app.getExecutor())
      mooseError("\"Executioner\" does not exist, make sure your input file contains an "
                 "[Executioner] block or your simulation adds an Executioner through an Action.");

    // This situation shouldn't be possible due to "determine_system_type" and/or autobuild.
    if (!_problem)
      mooseError("Your simulation does not contain a \"Problem\", which ironically means that YOU "
                 "have a problem...");
  }
  else
  {
    _awh.checkUnsatisfiedActions();

    mooseAssert(_problem, "Problem doesn't exist");
    _problem->checkProblemIntegrity();
  }
}

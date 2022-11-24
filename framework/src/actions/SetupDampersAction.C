//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SetupDampersAction.h"
#include "FEProblem.h"
#include "ActionWarehouse.h"

registerMooseAction("MooseApp", SetupDampersAction, "setup_dampers");

InputParameters
SetupDampersAction::validParams()
{
  InputParameters params = Action::validParams();
  return params;
}

SetupDampersAction::SetupDampersAction(const InputParameters & params) : Action(params) {}

void
SetupDampersAction::act()
{
  // if we have add_damper action, we will setup dampers ;-)
  ActionIterator it_beg = _awh.actionBlocksWithActionBegin("add_damper");
  ActionIterator it_end = _awh.actionBlocksWithActionEnd("add_damper");
  if (it_beg != it_end)
    _problem->setupDampers();
}

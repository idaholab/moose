//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddExecutorAction.h"
#include "FEProblem.h"
#include "BoundaryCondition.h"

registerMooseAction("MooseApp", AddExecutorAction, "add_executor");

InputParameters
AddExecutorAction::validParams()
{
  InputParameters params = Action::validParams();

  return params;
}

AddExecutorAction::AddExecutorAction(const InputParameters & params) : Action(params) {}

void
AddExecutorAction::act()
{
  _app.createExecutors();
}

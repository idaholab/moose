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

defineLegacyParams(AddExecutorAction);

InputParameters
AddExecutorAction::validParams()
{
  InputParameters params = Action::validParams();

  return params;
}

AddExecutorAction::AddExecutorAction(InputParameters params) : Action(params) {}

void
AddExecutorAction::act()
{
  std::cout<<"AddExecutorAction"<<std::endl;

  _app.createExecutors();
}

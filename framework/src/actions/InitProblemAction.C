//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InitProblemAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", InitProblemAction, "init_problem");

InputParameters
InitProblemAction::validParams()
{
  InputParameters params = Action::validParams();
  return params;
}

InitProblemAction::InitProblemAction(const InputParameters & params) : Action(params) {}

void
InitProblemAction::act()
{
  if (_problem.get())
    _problem->init();
  else
    mooseError("Problem doesn't exist in InitProblemAction!");
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "THMAddVariablesAction.h"
#include "THMProblem.h"

registerMooseAction("ThermalHydraulicsApp", THMAddVariablesAction, "THM:add_variables");

InputParameters
THMAddVariablesAction::validParams()
{
  InputParameters params = Action::validParams();
  return params;
}

THMAddVariablesAction::THMAddVariablesAction(const InputParameters & params) : Action(params) {}

void
THMAddVariablesAction::act()
{
  THMProblem * thm_problem = dynamic_cast<THMProblem *>(_problem.get());
  if (thm_problem)
    thm_problem->addVariables();
}

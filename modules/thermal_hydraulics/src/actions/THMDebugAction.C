//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "THMDebugAction.h"
#include "THMProblem.h"

registerMooseAction("ThermalHydraulicsApp", THMDebugAction, "THM:debug_action");

InputParameters
THMDebugAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addParam<bool>("check_jacobian", false, "Set to true to check jacobian");

  return params;
}

THMDebugAction::THMDebugAction(const InputParameters & params) : Action(params) {}

void
THMDebugAction::act()
{
  THMProblem * thm_problem = dynamic_cast<THMProblem *>(_problem.get());
  if (thm_problem)
    thm_problem->setCheckJacobian(getParam<bool>("check_jacobian"));
}

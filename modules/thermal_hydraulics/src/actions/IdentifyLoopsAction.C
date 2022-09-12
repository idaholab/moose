//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IdentifyLoopsAction.h"
#include "THMProblem.h"

registerMooseAction("ThermalHydraulicsApp", IdentifyLoopsAction, "THM:identify_loops");

InputParameters
IdentifyLoopsAction::validParams()
{
  InputParameters params = Action::validParams();

  return params;
}

IdentifyLoopsAction::IdentifyLoopsAction(const InputParameters & parameters) : Action(parameters) {}

void
IdentifyLoopsAction::act()
{
  THMProblem * thm_problem = dynamic_cast<THMProblem *>(_problem.get());
  if (thm_problem)
    thm_problem->identifyLoops();
}

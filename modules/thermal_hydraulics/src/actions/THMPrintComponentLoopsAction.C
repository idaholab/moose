//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "THMPrintComponentLoopsAction.h"
#include "THMProblem.h"

registerMooseAction("ThermalHydraulicsApp",
                    THMPrintComponentLoopsAction,
                    "THM:print_component_loops");

InputParameters
THMPrintComponentLoopsAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addParam<bool>("print_component_loops", false, "Flag to print component loops");

  return params;
}

THMPrintComponentLoopsAction::THMPrintComponentLoopsAction(const InputParameters & params)
  : Action(params)
{
}

void
THMPrintComponentLoopsAction::act()
{
  THMProblem * thm_problem = dynamic_cast<THMProblem *>(_problem.get());
  if (thm_problem && getParam<bool>("print_component_loops"))
    thm_problem->printComponentLoops();
}

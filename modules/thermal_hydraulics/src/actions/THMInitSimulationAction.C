//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "THMInitSimulationAction.h"
#include "THMProblem.h"

registerMooseAction("ThermalHydraulicsApp", THMInitSimulationAction, "THM:init_simulation");

InputParameters
THMInitSimulationAction::validParams()
{
  InputParameters params = Action::validParams();

  return params;
}

THMInitSimulationAction::THMInitSimulationAction(const InputParameters & parameters)
  : Action(parameters)
{
}

void
THMInitSimulationAction::act()
{
  THMProblem * thm_problem = dynamic_cast<THMProblem *>(_problem.get());
  if (thm_problem)
    thm_problem->initSimulation();
}

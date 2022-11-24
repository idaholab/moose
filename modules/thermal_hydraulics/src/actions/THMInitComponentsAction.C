//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "THMInitComponentsAction.h"
#include "THMProblem.h"

registerMooseAction("ThermalHydraulicsApp", THMInitComponentsAction, "THM:init_components");

InputParameters
THMInitComponentsAction::validParams()
{
  InputParameters params = Action::validParams();

  return params;
}

THMInitComponentsAction::THMInitComponentsAction(const InputParameters & parameters)
  : Action(parameters)
{
}

void
THMInitComponentsAction::act()
{
  THMProblem * thm_problem = dynamic_cast<THMProblem *>(_problem.get());
  if (thm_problem)
    thm_problem->initComponents();
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddComponentMooseObjectsAction.h"
#include "THMProblem.h"

registerMooseAction("ThermalHydraulicsApp",
                    AddComponentMooseObjectsAction,
                    "THM:add_component_moose_objects");

InputParameters
AddComponentMooseObjectsAction::validParams()
{
  InputParameters params = Action::validParams();
  return params;
}

AddComponentMooseObjectsAction::AddComponentMooseObjectsAction(const InputParameters & params)
  : Action(params)
{
}

void
AddComponentMooseObjectsAction::act()
{
  THMProblem * thm_problem = dynamic_cast<THMProblem *>(_problem.get());
  if (thm_problem)
    thm_problem->addMooseObjects();
}

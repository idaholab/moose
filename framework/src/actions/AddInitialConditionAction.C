//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddInitialConditionAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddInitialConditionAction, "add_ic");

InputParameters
AddInitialConditionAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add an InitialCondition object to the simulation.");
  return params;
}

AddInitialConditionAction::AddInitialConditionAction(const InputParameters & params)
  : MooseObjectAction(params)
{
}

void
AddInitialConditionAction::act()
{
  _problem->addInitialCondition(_type, _name, _moose_object_pars);
}

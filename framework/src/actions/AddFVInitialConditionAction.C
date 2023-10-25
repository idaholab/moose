//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddFVInitialConditionAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddFVInitialConditionAction, "add_fv_ic");

InputParameters
AddFVInitialConditionAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add an FVInitialCondition object to the simulation.");
  return params;
}

AddFVInitialConditionAction::AddFVInitialConditionAction(const InputParameters & params)
  : MooseObjectAction(params)
{
}

void
AddFVInitialConditionAction::act()
{
  _problem->addFVInitialCondition(_type, _name, _moose_object_pars);
}

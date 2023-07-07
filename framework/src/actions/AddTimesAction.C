//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddTimesAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddTimesAction, "add_times");

InputParameters
AddTimesAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a Times object to the simulation.");
  return params;
}

AddTimesAction::AddTimesAction(const InputParameters & params) : MooseObjectAction(params) {}

void
AddTimesAction::act()
{
  _problem->addReporter(_type, _name, _moose_object_pars);
}

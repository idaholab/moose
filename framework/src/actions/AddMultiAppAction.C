//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddMultiAppAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddMultiAppAction, "add_multi_app");

InputParameters
AddMultiAppAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a MultiApp object to the simulation.");
  return params;
}

AddMultiAppAction::AddMultiAppAction(const InputParameters & params) : MooseObjectAction(params) {}

void
AddMultiAppAction::act()
{
  _problem->addMultiApp(_type, _name, _moose_object_pars);
}

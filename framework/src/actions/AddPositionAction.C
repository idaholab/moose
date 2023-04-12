//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddPositionAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddPositionAction, "add_position");

InputParameters
AddPositionAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a Position object to the simulation.");
  return params;
}

AddPositionAction::AddPositionAction(const InputParameters & params) : MooseObjectAction(params) {}

void
AddPositionAction::act()
{
  _problem->addReporter(_type, _name, _moose_object_pars);
}

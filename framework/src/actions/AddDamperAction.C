//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddDamperAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddDamperAction, "add_damper");

InputParameters
AddDamperAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a Damper object to the simulation.");
  return params;
}

AddDamperAction::AddDamperAction(const InputParameters & params) : MooseObjectAction(params) {}

void
AddDamperAction::act()
{
  _problem->addDamper(_type, _name, _moose_object_pars);
}

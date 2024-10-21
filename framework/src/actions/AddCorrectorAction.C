//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddCorrectorAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddCorrectorAction, "add_corrector");

InputParameters
AddCorrectorAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a Corrector object to the simulation.");
  return params;
}

AddCorrectorAction::AddCorrectorAction(const InputParameters & params) : MooseObjectAction(params)
{
}

void
AddCorrectorAction::act()
{
  // Correctors are user objects in the backend
  _problem->addUserObject(_type, _name, _moose_object_pars);
}

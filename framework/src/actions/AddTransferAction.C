//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddTransferAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddTransferAction, "add_transfer");

InputParameters
AddTransferAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a Transfer object to the simulation.");
  return params;
}

AddTransferAction::AddTransferAction(const InputParameters & params) : MooseObjectAction(params) {}

void
AddTransferAction::act()
{
  _problem->addTransfer(_type, _name, _moose_object_pars);
}

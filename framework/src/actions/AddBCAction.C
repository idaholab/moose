//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddBCAction.h"
#include "FEProblem.h"
#include "BoundaryCondition.h"

registerMooseAction("MooseApp", AddBCAction, "add_bc");

InputParameters
AddBCAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a BoundaryCondition object to the simulation.");
  return params;
}

AddBCAction::AddBCAction(const InputParameters & params) : MooseObjectAction(params) {}

void
AddBCAction::act()
{
  _problem->addBoundaryCondition(_type, _name, _moose_object_pars);
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddConstraintAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddConstraintAction, "add_constraint");

InputParameters
AddConstraintAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a Constraint object to the simulation.");
  return params;
}

AddConstraintAction::AddConstraintAction(const InputParameters & params) : MooseObjectAction(params)
{
}

void
AddConstraintAction::act()
{
  _problem->addConstraint(_type, _name, _moose_object_pars);
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddIndicatorAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddIndicatorAction, "add_indicator");

InputParameters
AddIndicatorAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add an Indicator object to a simulation.");
  return params;
}

AddIndicatorAction::AddIndicatorAction(const InputParameters & params) : MooseObjectAction(params)
{
}

void
AddIndicatorAction::act()
{
  _problem->addIndicator(_type, _name, _moose_object_pars);
}

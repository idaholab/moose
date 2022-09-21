//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddDistributionAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddDistributionAction, "add_distribution");

InputParameters
AddDistributionAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a Distribution object to the simulation.");
  return params;
}

AddDistributionAction::AddDistributionAction(const InputParameters & params)
  : MooseObjectAction(params)
{
}

void
AddDistributionAction::act()
{
  _problem->addDistribution(_type, _name, _moose_object_pars);
}

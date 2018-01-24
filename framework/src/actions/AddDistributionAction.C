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

template <>
InputParameters
validParams<AddDistributionAction>()
{
  return validParams<MooseObjectAction>();
}

AddDistributionAction::AddDistributionAction(InputParameters params) : MooseObjectAction(params) {}

void
AddDistributionAction::act()
{
  _problem->addDistribution(_type, _name, _moose_object_pars);
}

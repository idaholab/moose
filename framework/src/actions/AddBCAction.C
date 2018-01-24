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

template <>
InputParameters
validParams<AddBCAction>()
{
  InputParameters params = validParams<MooseObjectAction>();
  params += validParams<BoundaryCondition>();
  return params;
}

AddBCAction::AddBCAction(InputParameters params) : MooseObjectAction(params) {}

void
AddBCAction::act()
{
  _problem->addBoundaryCondition(_type, _name, _moose_object_pars);
}

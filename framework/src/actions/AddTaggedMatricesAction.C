//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddTaggedMatricesAction.h"
#include "ActionWarehouse.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddTaggedMatricesAction, "create_tagged_matrices");

InputParameters
AddTaggedMatricesAction::validParams()
{
  return Action::validParams();
}

AddTaggedMatricesAction::AddTaggedMatricesAction(const InputParameters & params) : Action(params) {}

void
AddTaggedMatricesAction::act()
{
  _problem->createTagMatrices(FEProblemBase::CreateTaggedMatrixKey{});
}

//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddRemeshCriterionAction.h"

#include "FEProblem.h"

registerMooseAction("MooseApp", AddRemeshCriterionAction, "add_remesh_criterion");

InputParameters
AddRemeshCriterionAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a RemeshCriterion object to a simulation.");
  return params;
}

AddRemeshCriterionAction::AddRemeshCriterionAction(const InputParameters & params)
  : MooseObjectAction(params)
{
}

void
AddRemeshCriterionAction::act()
{
  _problem->addRemeshCriterion(_type, _name, _moose_object_pars);
}

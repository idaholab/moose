//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddRemesherAction.h"

#include "FEProblem.h"

registerMooseAction("MooseApp", AddRemesherAction, "add_remesher");

InputParameters
AddRemesherAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a Remesher object to a simulation.");
  return params;
}

AddRemesherAction::AddRemesherAction(const InputParameters & params) : MooseObjectAction(params) {}

void
AddRemesherAction::act()
{
  _problem->addRemesher(_type, _name, _moose_object_pars);
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddHybridizedBCAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddHybridizedBCAction, "add_bc");

InputParameters
AddHybridizedBCAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription(
      "Add a hybridized integrated boundary condition object to the simulation.");
  return params;
}

AddHybridizedBCAction::AddHybridizedBCAction(const InputParameters & params)
  : MooseObjectAction(params)
{
}

void
AddHybridizedBCAction::act()
{
  _problem->addHybridizedIntegratedBC(_type, _name, _moose_object_pars);
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddMaterialAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddMaterialAction, "add_material");

InputParameters
AddMaterialAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a Material object to the simulation.");
  return params;
}

AddMaterialAction::AddMaterialAction(const InputParameters & params) : MooseObjectAction(params) {}

void
AddMaterialAction::act()
{
  if (!_moose_object_pars.get<bool>("_interface"))
    _problem->addMaterial(_type, _name, _moose_object_pars);
  else
    _problem->addInterfaceMaterial(_type, _name, _moose_object_pars);
}

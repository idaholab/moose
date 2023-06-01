//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddFunctorMaterialAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddFunctorMaterialAction, "add_functor_material");

InputParameters
AddFunctorMaterialAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a Functor Material object to the simulation.");
  return params;
}

AddFunctorMaterialAction::AddFunctorMaterialAction(const InputParameters & params)
  : MooseObjectAction(params)
{
}

void
AddFunctorMaterialAction::act()
{
  // Functor materials are still materials in the back-end, for now
  _problem->addMaterial(_type, _name, _moose_object_pars);
}

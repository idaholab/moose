//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddKokkosMaterialAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddKokkosMaterialAction, "add_material");

InputParameters
AddKokkosMaterialAction::validParams()
{
  InputParameters params = KokkosObjectAction::validParams();
  params.addClassDescription("Add a Kokkos Material object to the simulation.");
  return params;
}

AddKokkosMaterialAction::AddKokkosMaterialAction(const InputParameters & params)
  : KokkosObjectAction(params, "Material")
{
}

void
AddKokkosMaterialAction::act()
{
  _problem->addKokkosMaterial(_type, _name, _moose_object_pars);
}

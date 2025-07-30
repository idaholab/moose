//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddGPUBCAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddKokkosBCAction, "add_bc");

InputParameters
AddKokkosBCAction::validParams()
{
  InputParameters params = KokkosObjectAction::validParams();
  params.addClassDescription("Add a Kokkos BoundaryCondition object to the simulation.");
  return params;
}

AddKokkosBCAction::AddKokkosBCAction(const InputParameters & params)
  : KokkosObjectAction(params, "BoundaryCondition")
{
}

void
AddKokkosBCAction::act()
{
  _problem->addKokkosBoundaryCondition(_type, _name, _moose_object_pars);
}

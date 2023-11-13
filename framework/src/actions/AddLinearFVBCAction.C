//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddLinearFVBCAction.h"
#include "FEProblem.h"
#include "NonlinearSystem.h"

registerMooseAction("MooseApp", AddLinearFVBCAction, "add_linear_fv_bc");

InputParameters
AddLinearFVBCAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a LinearFVBoundaryCondition object to the simulation.");
  return params;
}

AddLinearFVBCAction::AddLinearFVBCAction(const InputParameters & params) : MooseObjectAction(params)
{
}

void
AddLinearFVBCAction::act()
{
  if (_current_task == "add_linear_fv_bc")
    _problem->addLinearFVBC(_type, _name, _moose_object_pars);
}

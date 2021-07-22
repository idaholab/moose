//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddSolveObjectAction.h"

#include "Executioner.h"

registerMooseAction("MooseApp", AddSolveObjectAction, "add_solve_object");

InputParameters
AddSolveObjectAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a solve object to the executioner.");
  return params;
}

AddSolveObjectAction::AddSolveObjectAction(InputParameters params) : MooseObjectAction(params) {}

void
AddSolveObjectAction::act()
{
  auto executioner = _app.getExecutioner();
  if (!executioner)
    mooseError("Executioner must be created before adding solve objects");

  // skip several special names resolved for other actions
  if (_name == "TimeStepper" || _name == "TimeIntegrator" || _name == "Quadrature" ||
      _name == "Predictor" || _name == "Adaptivity")
    return;

  executioner->addSolveObject(_type, _name, _moose_object_pars);
}

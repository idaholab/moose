//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddFVBCAction.h"
#include "FEProblem.h"
#include "NonlinearSystem.h"

registerMooseAction("MooseApp", AddFVBCAction, "add_fv_bc");

defineLegacyParams(AddFVBCAction);

InputParameters
AddFVBCAction::validParams()
{
  return MooseObjectAction::validParams();
}

AddFVBCAction::AddFVBCAction(InputParameters params) : MooseObjectAction(params) {}

void
AddFVBCAction::act()
{
  if (_current_task == "add_fv_bc")
  {
    if (Registry::isADObj(_type + "<RESIDUAL>"))
    {
      _problem->addFVBC(_type + "<RESIDUAL>", _name + "_residual", _moose_object_pars);
      _problem->addFVBC(_type + "<JACOBIAN>", _name + "_jacobian", _moose_object_pars);
      _problem->haveADObjects(true);
    }
    else
      _problem->addFVBC(_type, _name, _moose_object_pars);
  }
}

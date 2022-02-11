//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVAction.h"
#include "MooseApp.h"
#include "TheWarehouse.h"
#include "INSFVAttributes.h"

registerMooseAction("NavierStokesApp", INSFVAction, "ns_meta_action");

InputParameters
INSFVAction::validParams()
{
  return Action::validParams();
}

INSFVAction::INSFVAction(InputParameters parameters) : Action(parameters) {}

void
INSFVAction::act()
{
  if (_current_task == "ns_meta_action")
  {
    _app.theWarehouse().registerAttribute<AttribINSFVBCs>("insfvbcs", 0);
    _app.theWarehouse().registerAttribute<AttribINSFVMomentumResidualObject>(
        "insfv_residual_object", false);
  }
}

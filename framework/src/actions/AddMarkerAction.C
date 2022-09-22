//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddMarkerAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddMarkerAction, "add_marker");

InputParameters
AddMarkerAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a Marker object to a simulation.");
  return params;
}

AddMarkerAction::AddMarkerAction(const InputParameters & params) : MooseObjectAction(params) {}

void
AddMarkerAction::act()
{
  _problem->addMarker(_type, _name, _moose_object_pars);
}

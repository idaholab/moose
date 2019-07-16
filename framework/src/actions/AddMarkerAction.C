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

template <>
InputParameters
validParams<AddMarkerAction>()
{
  InputParameters params = validParams<MooseObjectAction>();
  params.addClassDescription("Action for adding Marker object to a simulation.");
  return params;
}

AddMarkerAction::AddMarkerAction(InputParameters params) : MooseObjectAction(params) {}

void
AddMarkerAction::act()
{
  _problem->addMarker(_type, _name, _moose_object_pars);
}

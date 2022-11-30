//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "THMSetUpCoordinateSystemAction.h"
#include "THMProblem.h"

registerMooseAction("ThermalHydraulicsApp",
                    THMSetUpCoordinateSystemAction,
                    "THM:set_up_coordinate_system");

InputParameters
THMSetUpCoordinateSystemAction::validParams()
{
  InputParameters params = Action::validParams();
  params += THMAppInterface::validParams();

  params.addClassDescription("Sets up the coordinate system for THM.");

  return params;
}

THMSetUpCoordinateSystemAction::THMSetUpCoordinateSystemAction(const InputParameters & params)
  : Action(params), THMAppInterface(params)
{
}

void
THMSetUpCoordinateSystemAction::act()
{
  auto & thm_app = getTHMApp();
  if (thm_app.getComponents().size() > 0)
    thm_app.getTHMProblem().setupCoordinateSystem();
}

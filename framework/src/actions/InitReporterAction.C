//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InitReporterAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", InitReporterAction, "init_reporter_data");

InputParameters
InitReporterAction::validParams()
{
  return Action::validParams();
}

InitReporterAction::InitReporterAction(InputParameters params) : Action(params) {}

void
InitReporterAction::act()
{
  _problem->getReporterData().init();
}

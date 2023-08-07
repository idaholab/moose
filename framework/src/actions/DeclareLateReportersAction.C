//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DeclareLateReportersAction.h"
#include "MooseApp.h"
#include "Reporter.h"

registerMooseAction("MooseApp", DeclareLateReportersAction, "declare_late_reporters");

InputParameters
DeclareLateReportersAction::validParams()
{
  InputParameters params = Action::validParams();
  return params;
}

DeclareLateReportersAction::DeclareLateReportersAction(const InputParameters & params)
  : Action(params)
{
}

void
DeclareLateReportersAction::act()
{
  for (auto & reporter : _app.getInterfaceObjects<Reporter>())
    reporter->declareLateValues();
}

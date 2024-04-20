//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVActionDeprecation.h"

registerMooseAction("NavierStokesApp", NSFVActionDeprecation, "nsfv_action_deprecation_task");

InputParameters
NSFVActionDeprecation::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Helper class to smooth the deprecation of the NSFV Action");

  return params;
}

NSFVActionDeprecation::NSFVActionDeprecation(const InputParameters & parameters)
  : Action(parameters)
{
  mooseDeprecated(
      "The NSFVAction, used through the [Modules/NavierStokesFV] syntax is deprecated. "
      "Please convert your input file to using the [Physics/NavierStokes/WCNSFV...] syntax.");
}

void
NSFVActionDeprecation::act()
{
}

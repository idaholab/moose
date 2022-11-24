//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DeprecatedBlockAction.h"

registerMooseAction("MooseApp", DeprecatedBlockAction, "deprecated_block");

InputParameters
DeprecatedBlockAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Tool for marking input syntax as deprecated.");
  params.addParam<bool>("DEPRECATED", "*** WARNING: This block is deprecated - DO NOT USE ***");
  return params;
}

DeprecatedBlockAction::DeprecatedBlockAction(const InputParameters & parameters)
  : Action(parameters)
{
}

void
DeprecatedBlockAction::act()
{
  mooseError("Input file block '" + name() + "' has been deprecated.");
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DeprecatedBlockAction.h"

template <>
InputParameters
validParams<DeprecatedBlockAction>()
{
  InputParameters params = validParams<Action>();

  params.addParam<bool>("DEPRECATED", "*** WARNING: This block is deprecated - DO NOT USE ***");
  return params;
}

DeprecatedBlockAction::DeprecatedBlockAction(InputParameters parameters) : Action(parameters) {}

void
DeprecatedBlockAction::act()
{
  mooseError("Input file block '" + name() + "' has been deprecated.");
}

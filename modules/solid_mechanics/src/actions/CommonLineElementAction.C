//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CommonLineElementAction.h"
#include "LineElementAction.h"

registerMooseAction("TensorMechanicsApp", CommonLineElementAction, "meta_action");

InputParameters
CommonLineElementAction::validParams()
{
  InputParameters params = LineElementAction::validParams();
  return params;
}

CommonLineElementAction::CommonLineElementAction(const InputParameters & parameters)
  : Action(parameters)
{
}

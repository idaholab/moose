//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CommonShellElementAction.h"
#include "ShellElementAction.h"

registerMooseAction("TensorMechanicsApp", CommonShellElementAction, "meta_action");

InputParameters
CommonShellElementAction::validParams()
{
  InputParameters params = ShellElementAction::validParams();
  return params;
}

CommonShellElementAction::CommonShellElementAction(const InputParameters & parameters)
  : Action(parameters)
{
}

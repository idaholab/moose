//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EmptyAction.h"

registerMooseAction("MooseApp", EmptyAction, "no_action");

registerMooseAction("MooseApp", EmptyAction, "ready_to_init");

registerMooseAction("MooseApp", EmptyAction, "finish_input_file_output");

InputParameters
EmptyAction::validParams()
{
  InputParameters params = Action::validParams();
  return params;
}

EmptyAction::EmptyAction(const InputParameters & params) : Action(params) {}

void
EmptyAction::act()
{
}

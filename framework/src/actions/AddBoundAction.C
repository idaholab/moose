//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddBoundAction.h"

registerMooseAction("MooseApp", AddBoundAction, "add_bound");

InputParameters
AddBoundAction::validParams()
{
  InputParameters params = AddKernelAction::validParams();

  return params;
}

AddBoundAction::AddBoundAction(const InputParameters & params) : AddKernelAction(params) {}

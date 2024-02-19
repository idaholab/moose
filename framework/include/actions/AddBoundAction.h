//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObjectAction.h"
#include "AddKernelAction.h"

/**
 * Action to add Bounds objects within a Bounds block.
 */
class AddBoundAction : public AddKernelAction
{
public:
  static InputParameters validParams();

  AddBoundAction(const InputParameters & params);
};

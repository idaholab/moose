//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"

/**
 * This action tests two functions provided by action warehouse: getAction and getActions.
 * It uses AddMaterialAction as the example for calling these two functions.
 * These two functions allow an action to interfact with other actions.
 */
class TestGetActionsAction : public Action
{
public:
  static InputParameters validParams();

  TestGetActionsAction(const InputParameters & params);

  virtual void act() override;
};

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Action.h"

/**
 * Meta-action for creating common output object parameters
 * This action serves two purpose, first it adds common output object
 * parameters. Second, it creates the action (AddOutputAction) short-cuts
 * such as 'exodus=true' that result in the default output object of that
 * type to be created.
 * */
class CommonOutputAction : public Action
{
public:
  /**
   * Class constructor
   */
  static InputParameters validParams();

  CommonOutputAction(const InputParameters & params);

  virtual void act() override;

private:
  /**
   * Helper method for creating the short-cut actions
   * @param object_type String of the object type, i.e., the value of 'type=' in the input file
   */
  void create(std::string object_type);

  /**
   * Check if a Console object that outputs to the screen has been defined
   * @return True if the a screen outputting Console objects
   */
  bool hasConsole();

  /// Parameters from the action being created (AddOutputAction)
  InputParameters _action_params;
};

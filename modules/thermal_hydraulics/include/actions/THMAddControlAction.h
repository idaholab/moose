//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AddControlAction.h"

/**
 * Action for adding THM control objects
 */
class THMAddControlAction : public AddControlAction
{
public:
  /**
   * Class constructor
   * @param params Parameters for this Action
   */
  THMAddControlAction(const InputParameters & parameters);

  virtual void act() override;

public:
  static InputParameters validParams();
};

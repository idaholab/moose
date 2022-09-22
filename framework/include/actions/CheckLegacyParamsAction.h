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
 * Action for checking that the registered objects
 * do not use the legacy input parameter construction
 *
 * Needed until #19439 is closed
 */
class CheckLegacyParamsAction : public Action
{
public:
  CheckLegacyParamsAction(const InputParameters & params);

  static InputParameters validParams();

  virtual void act() override;
};

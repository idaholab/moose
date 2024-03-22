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
 * This class helps with the deprecation of this syntax
 * [Modules]
 *   [NavierStokesFV]
 *     param_1 = value_1
 *     param_2 = value_2
 *     ...
 *   []
 * []
 */
class NSFVActionDeprecation : public Action
{
public:
  static InputParameters validParams();

  NSFVActionDeprecation(const InputParameters & parameters);

  virtual void act() override;
};

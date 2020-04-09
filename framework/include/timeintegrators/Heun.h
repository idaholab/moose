//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ExplicitRK2.h"

/**
 * Heun's (aka improved Euler) time integration method.
 *
 * The Butcher tableau for this method is:
 * 0   | 0
 * 1   | 1    0
 * ---------------------
 *     | 1/2  1/2
 *
 * See: ExplicitRK2.h for more information.
 */
class Heun : public ExplicitRK2
{
public:
  static InputParameters validParams();

  Heun(const InputParameters & parameters);

protected:
  /// Method coefficient overrides
  virtual Real a() const { return 1.; }
  virtual Real b1() const { return .5; }
  virtual Real b2() const { return .5; }
};

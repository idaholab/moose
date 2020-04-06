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
 * Ralston's time integration method.
 *
 * The Butcher tableau for this method is:
 * 0   | 0
 * 2/3 | 2/3    0
 * ---------------------
 *     | 1/4  3/4
 *
 * See: ExplicitRK2.h for more information.
 */
class Ralston : public ExplicitRK2
{
public:
  static InputParameters validParams();

  Ralston(const InputParameters & parameters);

protected:
  /// Method coefficient overrides
  virtual Real a() const { return 2. / 3.; }
  virtual Real b1() const { return .25; }
  virtual Real b2() const { return .75; }
};

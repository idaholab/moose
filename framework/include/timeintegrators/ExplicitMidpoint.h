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
 * The explicit midpoint time integration method.
 *
 * The Butcher tableau for this method is:
 * 0   | 0
 * 1/2 | 1/2 0
 * ---------------------
 *     | 0   1
 *
 * See: ExplicitRK2.h for more information.
 */
class ExplicitMidpoint : public ExplicitRK2
{
public:
  static InputParameters validParams();

  ExplicitMidpoint(const InputParameters & parameters);
  virtual ~ExplicitMidpoint() {}

protected:
  /// Method coefficient overrides
  virtual Real a() const { return .5; }
  virtual Real b1() const { return 0.; }
  virtual Real b2() const { return 1.; }
};

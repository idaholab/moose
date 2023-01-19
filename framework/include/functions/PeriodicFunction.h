//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Function.h"
#include "FunctionInterface.h"

class PeriodicFunction : public Function, protected FunctionInterface
{
public:
  static InputParameters validParams();
  PeriodicFunction(const InputParameters & parameters);

  virtual Real value(Real t, const Point & p) const override;

private:
  /// Function used as a basis for the periodic function
  const Function & _base_function;

  /// Period for repetition of the base function in time
  const Real _period_time;

  /// Period for repetition of the base function in the x direction
  const Real _period_x;

  /// Period for repetition of the base function in the y direction
  const Real _period_y;

  /// Period for repetition of the base function in the z direction
  const Real _period_z;
};

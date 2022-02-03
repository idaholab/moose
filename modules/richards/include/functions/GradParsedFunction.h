//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseParsedFunction.h"

// Forward declarations

/**
 * returns the central difference approx to the derivative
 * of the function, ie
 * (f(t, p + direction) - f(t, p - direction))/2/|direction|
 * This derives from MooseParsedFunction, so it already knows about a function
 */
class GradParsedFunction : public MooseParsedFunction
{
public:
  static InputParameters validParams();

  GradParsedFunction(const InputParameters & parameters);

  using Function::value;
  virtual Real value(Real t, const Point & pt) const;

protected:
  /// central difference direction
  RealVectorValue _direction;

  /// 2*|_direction|
  Real _len;
};

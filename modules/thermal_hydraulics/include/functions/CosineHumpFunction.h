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

/**
 * Computes a cosine hump of a user-specified width and height
 *
 * Currently it is assumed that the direction of the hump is in the x, y,
 * or z direction, but this could later be extended to an arbitrary direction.
 */
class CosineHumpFunction : public Function
{
public:
  CosineHumpFunction(const InputParameters & parameters);

  using Function::value;
  virtual Real value(Real t, const Point & p) const;
  virtual RealVectorValue gradient(Real t, const Point & p) const;

protected:
  /// Component index of axis on which hump occurs
  const unsigned int _component;
  /// Width of hump
  const Real & _hump_width;
  /// Hump center position on selected axis
  const Real & _hump_center_position;

  /// Value before and after the hump
  const Real & _hump_begin_value;
  /// Value at the center of the hump
  const Real & _hump_center_value;

  /// Cosine amplitude
  const Real _cosine_amplitude;
  /// Middle value of hump
  const Real _hump_mid_value;
  /// Left end of hump
  const Real _hump_left_end;
  /// Right end of hump
  const Real _hump_right_end;

public:
  static InputParameters validParams();
};

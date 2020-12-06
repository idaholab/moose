//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InitialCondition.h"

/**
 * ThumbIC creates a rectangle with a half circle on top
 */
class ThumbIC : public InitialCondition
{
public:
  static InputParameters validParams();

  ThumbIC(const InputParameters & parameters);

  virtual Real value(const Point & p);

protected:
  const Real _xcoord;
  const Real _width;
  const Real _height;
  const Real _invalue;
  const Real _outvalue;
};

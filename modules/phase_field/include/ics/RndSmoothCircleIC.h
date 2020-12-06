//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SmoothCircleIC.h"

/**
 * RndSmoothcircleIC creates a smooth circle with a random distribution
 * of values inside and outside of the circle.
 */
class RndSmoothCircleIC : public SmoothCircleIC
{
public:
  static InputParameters validParams();

  RndSmoothCircleIC(const InputParameters & parameters);

private:
  virtual Real computeCircleValue(const Point & p, const Point & center, const Real & radius);

  const Real _variation_invalue;
  const Real _variation_outvalue;
};

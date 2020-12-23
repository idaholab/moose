//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SmoothCircleBaseIC.h"

/**
 * SmoothcircleIC creates a circle of a given radius centered at a given point in the domain.
 * If int_width > zero, the border of the circle with smoothly transition from
 * the invalue to the outvalue.
 */
class SmoothCircleIC : public SmoothCircleBaseIC
{
public:
  static InputParameters validParams();

  SmoothCircleIC(const InputParameters & parameters);

protected:
  virtual void computeCircleRadii();
  virtual void computeCircleCenters();

  Real _x1;
  Real _y1;
  Real _z1;
  Real _radius;
  Point _center;
};

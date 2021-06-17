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
 * RndBoundingBoxIC allows setting the initial condition of a value inside and outside of a
 * specified box.
 * The box is aligned with the x,y,z axis... and is specified by passing in the x,y,z coordinates of
 * the bottom
 * left point and the top right point. Each of the coordinates of the "bottom_left" point MUST be
 * less than
 * those coordinates in the "top_right" point.
 *
 * When setting the initial condition if bottom_left <= Point <= top_right then the "inside" value
 * is used.
 * Otherwise the "outside" value is used.
 */
class RndBoundingBoxIC : public InitialCondition
{
public:
  static InputParameters validParams();

  RndBoundingBoxIC(const InputParameters & parameters);

  virtual Real value(const Point & p);

private:
  const Real _x1;
  const Real _y1;
  const Real _z1;
  const Real _x2;
  const Real _y2;
  const Real _z2;
  const Real _mx_invalue;
  const Real _mx_outvalue;
  const Real _mn_invalue;
  const Real _mn_outvalue;
  const Real _range_invalue;
  const Real _range_outvalue;

  const Point _bottom_left;
  const Point _top_right;
};

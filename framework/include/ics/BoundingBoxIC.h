//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef BOUNDINGBOXIC_H
#define BOUNDINGBOXIC_H

#include "InitialCondition.h"

// System includes
#include <string>

// Forward Declarations
class BoundingBoxIC;
class InputParameters;

namespace libMesh
{
class Point;
}

template <typename T>
InputParameters validParams();

template <>
InputParameters validParams<BoundingBoxIC>();

/**
 * BoundingBoxIC allows setting the initial condition of a value inside and outside of a specified
 * box.
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
class BoundingBoxIC : public InitialCondition
{
public:
  /**
   * Constructor
   *
   * @param parameters The parameters object holding data for the class to use.
   */
  BoundingBoxIC(const InputParameters & parameters);

  /**
   * The value of the variable at a point.
   *
   * This must be overridden by derived classes.
   */
  virtual Real value(const Point & p);

protected:
  Real _x1;
  Real _y1;
  Real _z1;
  Real _x2;
  Real _y2;
  Real _z2;
  Real _inside;
  Real _outside;

  Point _bottom_left;
  Point _top_right;
};

#endif // BOUNDINGBOXIC_H

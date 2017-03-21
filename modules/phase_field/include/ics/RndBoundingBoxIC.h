/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef RNDBOUNDINGBOXIC_H
#define RNDBOUNDINGBOXIC_H

#include "InitialCondition.h"

// Forward Declarations
class RndBoundingBoxIC;

template <>
InputParameters validParams<RndBoundingBoxIC>();

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

#endif // RNDBOUNDINGBOXIC_H

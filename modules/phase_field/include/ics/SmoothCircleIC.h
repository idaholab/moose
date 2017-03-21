/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SMOOTHCIRCLEIC_H
#define SMOOTHCIRCLEIC_H

#include "SmoothCircleBaseIC.h"

// Forward Declarations
class SmoothCircleIC;

template <>
InputParameters validParams<SmoothCircleIC>();

/**
 * SmoothcircleIC creates a circle of a given radius centered at a given point in the domain.
 * If int_width > zero, the border of the circle with smoothly transition from
 * the invalue to the outvalue.
 */
class SmoothCircleIC : public SmoothCircleBaseIC
{
public:
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

#endif // SMOOTHCIRCLEIC_H

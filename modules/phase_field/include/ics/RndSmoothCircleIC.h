/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef RNDSMOOTHCIRCLEIC_H
#define RNDSMOOTHCIRCLEIC_H

#include "SmoothCircleIC.h"

// Forward Declarations
class RndSmoothCircleIC;

template <>
InputParameters validParams<RndSmoothCircleIC>();

/**
 * RndSmoothcircleIC creates a smooth circle with a random distribution
 * of values inside and outside of the circle.
 */
class RndSmoothCircleIC : public SmoothCircleIC
{
public:
  RndSmoothCircleIC(const InputParameters & parameters);

private:
  virtual Real computeCircleValue(const Point & p, const Point & center, const Real & radius);

  const Real _variation_invalue;
  const Real _variation_outvalue;
};

#endif // RNDSMOOTHCIRCLEIC_H

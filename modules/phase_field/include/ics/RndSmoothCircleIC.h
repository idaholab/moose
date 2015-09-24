/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef RNDSMOOTHCIRCLEIC_H
#define RNDSMOOTHCIRCLEIC_H

#include "Kernel.h"
#include "SmoothCircleIC.h"

// System includes
#include <string>

// Forward Declarations
class RndSmoothCircleIC;

template<>
InputParameters validParams<RndSmoothCircleIC>();

/**
 * RndSmoothcircleIC creates a smooth circle with a random distribution
 * of values inside and outside of the circle.
 **/
class RndSmoothCircleIC : public SmoothCircleIC
{
public:
  /**
   * Constructor
   *
   * @param parameters The parameters object holding data for the class to use.
   */
  RndSmoothCircleIC(const InputParameters & parameters);

  /**
   * The value of the variable at a point.
   *
   * This must be overriden by derived classes.
   */
  virtual Real computeCircleValue(const Point & p, const Point & center, const Real & radius);

private:
  Real _variation_invalue;
  Real _variation_outvalue;
};

#endif //RNDSMOOTHCIRCLEIC_H

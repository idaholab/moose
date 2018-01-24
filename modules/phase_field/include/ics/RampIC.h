/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef RAMPIC_H
#define RAMPIC_H

#include "InitialCondition.h"

// Forward Declarations
class RampIC;

template <>
InputParameters validParams<RampIC>();

/**
 * Makes initial condition which creates a linear ramp of the given variable
 * on the x-axis with specified side values
 */
class RampIC : public InitialCondition
{
public:
  RampIC(const InputParameters & parameters);

protected:
  /**
   * The value of the variable at a point.
   */
  virtual Real value(const Point & p);

  /**
   * The value of the gradient at a point.
   */
  virtual RealGradient gradient(const Point & /*p*/);

  const Real _xlength;
  const Real _xmin;
  const Real _value_left;
  const Real _value_right;
};

#endif // RAMPIC_H

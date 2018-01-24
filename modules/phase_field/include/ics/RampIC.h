//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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

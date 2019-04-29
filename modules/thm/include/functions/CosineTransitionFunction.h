#pragma once

#include "Function.h"

class CosineTransitionFunction;

template <>
InputParameters validParams<CosineTransitionFunction>();

/**
 * Computes a cosine transtition of a user-specified width between two values
 *
 * Currently it is assumed that the direction of the transition is in time or in
 * the x, y, z direction, but this could later be extended to an arbitrary
 * direction.
 */
class CosineTransitionFunction : public Function
{
public:
  CosineTransitionFunction(const InputParameters & parameters);

  virtual Real value(Real t, const Point & p);
  virtual RealVectorValue gradient(Real t, const Point & p);

protected:
  /// Component index of axis on which transition occurs
  const unsigned int _component;
  /// Use the time axis for transition?
  const bool _use_time;
  /// Width of transition region
  const Real & _transition_width;
  /// Coordinate of beginning of transition
  const Real & _begin_coordinate;
  /// Coordinate of end of transition
  const Real _end_coordinate;

  /// Value at beginning of transition
  const Real & _begin_value;
  /// Value at end of transition
  const Real & _end_value;
  /// Amplitude of the cosine function
  const Real _amplitude;
};

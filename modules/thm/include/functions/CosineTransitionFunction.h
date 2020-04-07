#pragma once

#include "SmoothTransitionFunction.h"
#include "WeightedTransition.h"

/**
 * Computes a cosine transtition of a user-specified width between two values
 *
 * Currently it is assumed that the direction of the transition is in time or in
 * the x, y, z direction, but this could later be extended to an arbitrary
 * direction.
 */
class CosineTransitionFunction : public SmoothTransitionFunction
{
public:
  CosineTransitionFunction(const InputParameters & parameters);

  virtual Real value(Real t, const Point & p) const;
  virtual RealVectorValue gradient(Real t, const Point & p) const;

protected:
  /// Transition object
  const WeightedTransition _transition;

public:
  static InputParameters validParams();
};

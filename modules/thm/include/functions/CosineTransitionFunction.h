#pragma once

#include "SmoothTransitionFunction.h"
#include "WeightedTransitionInterface.h"

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
class CosineTransitionFunction : public SmoothTransitionFunction, public WeightedTransitionInterface
{
public:
  CosineTransitionFunction(const InputParameters & parameters);

  virtual Real value(Real t, const Point & p) const;
  virtual RealVectorValue gradient(Real t, const Point & p) const;
};

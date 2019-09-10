#pragma once

#include "MooseTypes.h"
#include "InputParameters.h"

class SmoothTransitionInterface;

template <>
InputParameters validParams<SmoothTransitionInterface>();

/**
 * Base interface class for smooth transitions between two functions of one variable
 */
class SmoothTransitionInterface
{
public:
  SmoothTransitionInterface(const MooseObject * moose_object);

protected:
  /**
   * Computes the transition value
   *
   * @param[in] x    Point at which to evaluate function
   * @param[in] f1   Left function
   * @param[in] f2   Right function
   */
  virtual Real computeTransitionValue(const Real & x, const Real & f1, const Real & f2) const = 0;

  /// Center point of transition
  const Real & _x_center;
  /// Width of transition
  const Real & _transition_width;

  /// Left end point of transition
  const Real _x1;
  /// Right end point of transition
  const Real _x2;
};

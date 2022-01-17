#pragma once

#include "MooseTypes.h"

/**
 * Base class for smooth transitions between two functions of one variable
 */
class SmoothTransition
{
public:
  /**
   * Constructor.
   *
   * @param[in] x_center   Center point of transition
   * @param[in] transition_width   Width of transition
   */
  SmoothTransition(const Real & x_center, const Real & transition_width);

  /**
   * Computes the transition value
   *
   * @param[in] x    Point at which to evaluate function
   * @param[in] f1   Left function
   * @param[in] f2   Right function
   */
  virtual Real value(const Real & x, const Real & f1, const Real & f2) const = 0;

  /**
   * Returns the coordinate of the left end of the transition
   */
  const Real & leftEnd() const { return _x1; }

  /**
   * Returns the coordinate of the right end of the transition
   */
  const Real & rightEnd() const { return _x2; }

protected:
  /// Center point of transition
  const Real _x_center;
  /// Width of transition
  const Real _transition_width;

  /// Left end point of transition
  const Real _x1;
  /// Right end point of transition
  const Real _x2;
};

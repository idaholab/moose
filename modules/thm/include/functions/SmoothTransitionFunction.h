#pragma once

#include "Function.h"
#include "FunctionInterface.h"

/**
 * Base class for functions to smoothly transition from one function to another
 *
 * Currently it is assumed that the direction of the transition is in time or in
 * the x, y, z direction, but this could later be extended to an arbitrary
 * direction.
 */
class SmoothTransitionFunction : public Function, public FunctionInterface
{
public:
  SmoothTransitionFunction(const InputParameters & parameters);

protected:
  /// Component index of axis on which transition occurs
  const unsigned int _component;
  /// Use the time axis for transition?
  const bool _use_time;

  /// First function
  const Function & _function1;
  /// Second function
  const Function & _function2;

  /// Center point of transition
  const Real & _x_center;
  /// Width of transition
  const Real & _transition_width;

public:
  static InputParameters validParams();
};

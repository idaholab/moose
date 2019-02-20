#ifndef TIMERAMPFUNCTION_H
#define TIMERAMPFUNCTION_H

#include "Function.h"

class TimeRampFunction;

template <>
InputParameters validParams<TimeRampFunction>();

/**
 * Ramps up to a value from another value over time.
 *
 * Examples of how this is intended to be used include the time step size,
 * which may need to be smaller at the beginning of a transient, and boundary
 * condition parameters, which may need to start closer to the steady-state
 * values at the beginning of a transient.
 */
class TimeRampFunction : public Function
{
public:
  TimeRampFunction(const InputParameters & parameters);

  virtual Real value(Real t, const Point & p);
  virtual RealVectorValue gradient(Real t, const Point & p);

protected:
  /// Initial value
  const Real & _initial_value;
  /// Final value
  const Real & _final_value;
  /// Ramp duration
  const Real & _ramp_duration;
  /// Initial time
  const Real & _initial_time;
  /// Ramp slope
  const Real _ramp_slope;
};

#endif // TIMERAMPFUNCTION_H

#ifndef PIDCONTROL_H
#define PIDCONTROL_H

#include "RELAP7Control.h"

class PIDControl;

template <>
InputParameters validParams<PIDControl>();

/**
 * This block represents a proportional–integral–derivative controller (PID controller). It
 * continuously calculates an error value e(t) as the difference between a desired setpoint and a
 * measured process variable and applies a correction based on proportional, integral, and
 * derivative terms.
 */
class PIDControl : public RELAP7Control
{
public:
  PIDControl(const InputParameters & parameters);

  virtual void execute();

protected:
  /// input data
  const Real & _value;
  /// set point
  const Real & _set_point;
  /// The coefficient for the proportional term
  const Real & _K_p;
  /// The coefficient for the integral term
  const Real & _K_i;
  /// The coefficient for the derivative term
  const Real & _K_d;
  /// The output computed by the PID controller
  Real & _output;
  /// The integral value accumulated over time
  Real & _integral;
  /// The old value of the error
  Real _error_old;
};

#endif // PIDCONTROL_H

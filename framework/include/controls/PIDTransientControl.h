//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Control.h"

/**
 * A time-dependent control of an input parameter or a postprocessor, which aims at
 * making a postprocessor match a desired value.
 */
class PIDTransientControl : public Control
{
public:
  static InputParameters validParams();

  /**
   * Class constructor
   * @param parameters Input parameters for this Control object
   */
  PIDTransientControl(const InputParameters & parameters);

  virtual void execute() override;

private:
  /// The current value of the target postprocessor
  const PostprocessorValue & _current;
  /// The target 1D time-dependent function for the postprocessor
  const Function & _target;
  /// The coefficient multiplying the integral of the error
  const Real _Kint;
  /// The coefficient multiplying the error
  const Real _Kpro;
  /// The coefficient multiplying the derivative of the error
  const Real _Kder;
  /// The time to start the PID controller on
  const Real _start_time;
  /// The time to stop using the PID controller on
  const Real _stop_time;
  /// Whether to reset the PID integral error when changing timestep, to limit its action to within coupling iterations
  const bool _reset_every_timestep;
  /// Whether to reset the PID integral error when the error crosses 0, to avoid windup
  const bool _reset_integral_windup;
  /// Limiting maximum value for the output of the PID controller
  const Real _maximum_output_value;
  /// Limiting minimum value for the output of the PID controller
  const Real _minimum_output_value;
  /// Limiting maximum value for the rate of change of output of the PID controller
  const Real _maximum_change_rate;
  /// Integral of the error
  Real & _integral;
  /// Saved value of the integral at the beginning of a timestep, to recover from a failed solve
  Real & _integral_old;
  /// Saved value of the controlled parameter at the end of a timestep. This is recomputed every time, but need to be restarted
  /// for recover simulations
  Real & _value;
  /// Saved value of the controlled parameter at the beginning of a timestep, to recover from a failed solve
  Real & _value_old;
  /// the previous time step
  int & _t_step_old;
  /// the previous value of the difference with the target, to detect changes of sign, and to compute the derivative term
  Real & _old_delta;
  /// whether the app has recovered once, because the logic for setting the value is different after having just recovered
  bool _has_recovered;
  /// the difference with the target from the previous time step, used if a time step fails
  Real _delta_prev_tstep = 0.0;
  /// the difference with the target from the from the second-to-last iteration of previous time step, used if a time step fails
  Real _old_delta_prev_tstep = 0.0;
};

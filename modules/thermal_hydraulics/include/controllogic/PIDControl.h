//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "THMControl.h"

/**
 * This block represents a proportional-integral-derivative controller (PID controller). It
 * continuously calculates an error value e(t) as the difference between a desired setpoint and a
 * measured process variable and applies a correction based on proportional, integral, and
 * derivative terms.
 */
class PIDControl : public THMControl
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
  /// Initial value
  const Real & _initial_value;
  /// The integral value accumulated over time
  Real & _integral;
  /// The old value of _integral
  const Real & _integral_old;
  /// The current value of the error
  Real & _error;
  /// The old value of the error
  const Real & _error_old;

public:
  static InputParameters validParams();
};

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ChainControl.h"

/**
 * Implements a proportional-integral-derivative (PID) controller.
 */
class PIDChainControl : public ChainControl
{
public:
  static InputParameters validParams();

  PIDChainControl(const InputParameters & parameters);

  virtual void execute() override;

protected:
  /// Updates all control data values
  void updateValues();

  /// input data
  const Real & _input;
  /// set point
  const Real & _set_point;
  /// The coefficient for the proportional term
  const Real & _K_p;
  /// The coefficient for the integral term
  const Real & _K_i;
  /// The coefficient for the derivative term
  const Real & _K_d;

  /// The current value of the error
  Real & _error;
  /// The old value of the error
  const Real & _error_old;

  /// The proportional component
  Real & _proportional;
  /// The integral component
  Real & _integral;
  /// The old value of \c _integral
  const Real & _integral_old;
  /// The derivative component
  Real & _derivative;

  /// The output computed by the PID controller
  Real & _output;

  /// Previous time for which value was computed
  Real & _previous_time;
};

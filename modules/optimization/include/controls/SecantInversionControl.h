//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InversionControlBase.h"

/**
 * Control that adjusts a scalar parameter (held in a postprocessor) once per fixed-point
 * iteration using the secant method, so that a sub-app output postprocessor matches a target
 * function of time. The outer iteration, convergence, and time-step cutting are owned by the
 * executioner and the Convergence system; this object only performs the secant update.
 */
class SecantInversionControl : public InversionControlBase
{
public:
  static InputParameters validParams();

  SecantInversionControl(const InputParameters & parameters);

  virtual void execute() override;

private:
  /// Perturbation applied on the first iteration of each sweep to seed the secant method
  const Real _initial_delta;
  /// Previous iteration's parameter value (secant point); refreshed each sweep
  Real _p_prev;
  /// Previous iteration's output value (secant point); refreshed each sweep
  Real _y_prev;
};

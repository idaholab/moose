//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TimeStepper.h"

/**
 * Computes time step size based on a target number of fixed point iterations
 */
class FixedPointIterationAdaptiveDT : public TimeStepper
{
public:
  static InputParameters validParams();

  FixedPointIterationAdaptiveDT(const InputParameters & parameters);

  virtual void init() override;
  virtual void acceptStep() override;

protected:
  virtual Real computeInitialDT() override;
  virtual Real computeDT() override;

  /// Initial time step size
  const Real _dt_initial;

  /// Target number of fixed point iterations
  const unsigned int _target_center;
  /// Number subtracted/added to determine window min and max
  const unsigned int _target_window;
  /// Minimum fixed point iterations of window
  const unsigned int _target_min;
  /// Maximum fixed point iterations of window
  const unsigned int _target_max;

  /// Factor by which to increase time steps
  const Real _increase_factor;
  /// Factor by which to decrease time steps
  const Real _decrease_factor;

  /// Old time step size
  Real & _dt_old;
  /// Number of fixed point iterations in previous solve
  unsigned int & _fp_its;
};

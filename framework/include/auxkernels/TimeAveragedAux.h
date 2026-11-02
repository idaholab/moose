//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

/**
 * Running (time-weighted) mean of a functor (FE, FV or linear FV variable, or any functor),
 * updated every time it executes:
 *
 *   mean^n = mean^{n-1} + (u^n - mean^{n-1}) * dt / (t - t_start)
 *
 * With time_integration_scheme = trapezoidal-rule, u^n is replaced by 0.5 * (u^n + u^{n-1}),
 * the trapezoidal rule over the time step. time-integrator selects the trapezoidal rule when
 * every time integrator of the executioner is at least second order, and the rectangle rule
 * otherwise.
 *
 * Averaging starts at the first time step with t >= start_time. Before that, the variable
 * holds the instantaneous value. The update only uses the old value of the variable, so
 * executing more than once per time step gives the same result. Restartable.
 *
 * This could be later expanded to also compute standard deviations and higher order stats.
 */
class TimeAveragedAux : public AuxKernel
{
public:
  static InputParameters validParams();

  TimeAveragedAux(const InputParameters & parameters);

  virtual void timestepSetup() override;

protected:
  virtual Real computeValue() override;

  /// Whether the time integration scheme selected by the user is the trapezoidal rule
  bool useTrapezoidalRule() const;

  /// Quantity being averaged
  const Moose::Functor<Real> & _functor;

  /// Value of the average at the previous time step
  const VariableValue & _mean_old;

  /// Time at which averaging may start
  const Real _start_time;

  /// Whether the average is integrated with the trapezoidal rule rather than the rectangle rule
  const bool _trapezoidal;

  /// Restartable state
  bool & _averaging;
  Real & _t_start;
};

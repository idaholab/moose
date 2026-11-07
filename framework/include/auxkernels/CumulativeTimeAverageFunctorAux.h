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
 * Running (time-weighted) mean of a functor (FE, FV or linear FV variable, or any functor)
 * over [t_start, t], updated every time it executes:
 *
 *   mean^n = mean^{n-1} + (u^n - mean^{n-1}) * w / (t - t_start)
 *
 * where t_start = max(start_time, beginning of the first time step with t >= start_time), so
 * that no time before start_time is included, and w = min(dt, t - t_start) is the part of the
 * time step after t_start.
 * With time_integration_scheme = trapezoidal-rule, u^n is replaced by the trapezoidal rule over
 * that part of the time step, with u at its beginning linearly interpolated between u^{n-1}
 * and u^n.
 *
 * Averaging starts at the first time step with t >= start_time. Before that, and at
 * t = start_time, the variable holds the instantaneous value. The update only uses the old value
 * of the variable, so executing more than once per time step gives the same result. Restartable.
 *
 * This could be later expanded to also compute standard deviations and higher order stats.
 */
class CumulativeTimeAverageFunctorAux : public AuxKernel
{
public:
  static InputParameters validParams();

  CumulativeTimeAverageFunctorAux(const InputParameters & parameters);

  virtual void timestepSetup() override;

protected:
  virtual Real computeValue() override;

  /// Quadratures of the time integral of the functor over a time step, in the order of the
  /// time_integration_scheme MooseEnum options so that MooseEnum::getEnum maps onto them
  enum class Scheme
  {
    IMPLICIT_EULER,
    TRAPEZOIDAL_RULE
  };

  /// Quantity being averaged
  const Moose::Functor<Real> & _functor;

  /// Value of the average at the previous time step
  const VariableValue & _mean_old;

  /// Time at which averaging starts
  const Real _start_time;

  /// Quadrature of the time integral of the functor over a time step
  const Scheme _scheme;

  /// Restartable state
  bool & _averaging;
  /// Beginning of the averaging interval, never earlier than start_time
  Real & _t_start;
};

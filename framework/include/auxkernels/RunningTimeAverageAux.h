#pragma once

#include "AuxKernel.h"

/**
 * Running (time-weighted) mean of a functor (FE, FV or linear FV variable, or any functor),
 * updated every time it executes:
 *
 *   mean^n = mean^{n-1} + (u^n - mean^{n-1}) * dt / (t - t_start)
 *
 * Averaging starts at the first time step with t >= start_time. Before that, the variable
 * holds the instantaneous value. The update only uses the old value of the variable, so
 * executing more than once per time step gives the same result. Restartable.
 *
 * This could be later expanded to also compute standard deviations and higher order stats.
 */
class RunningTimeAverageAux : public AuxKernel
{
public:
  static InputParameters validParams();

  RunningTimeAverageAux(const InputParameters & parameters);

  virtual void timestepSetup() override;

protected:
  virtual Real computeValue() override;

  /// Quantity being averaged
  const Moose::Functor<Real> & _functor;

  /// Value of the average at the previous time step
  const VariableValue & _mean_old;

  /// Time at which averaging may start
  const Real _start_time;

  /// Restartable state
  bool & _averaging;
  Real & _t_start;
};

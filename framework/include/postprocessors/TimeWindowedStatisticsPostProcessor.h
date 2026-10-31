#pragma once

#include "GeneralPostprocessor.h"

#include <vector>

/**
 * Time statistics of another postprocessor, using only samples with t >= start_time.
 *
 *   mean            : mean over the last `window` steps
 *   std             : sample standard deviation over the last `window` steps
 *   cumulative_mean : time-weighted running mean since start_time
 *   averaging_time  : time elapsed since averaging started
 *
 * Before start_time: mean and cumulative_mean return the instantaneous value,
 * std and averaging_time return 0, relative_drift returns 1.
 *
 * Averaging starts at the first time step with t >= start_time. One sample is recorded
 * per time step, so use the default execute_on = TIMESTEP_END. All history is restartable.
 */
class TimeWindowedStatisticsPostProcessor : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  TimeWindowedStatisticsPostProcessor(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}
  virtual Real getValue() const override { return _value; }

protected:
  /// Mean of _history over [begin, end)
  Real windowMean(std::size_t begin, std::size_t end) const;

  const PostprocessorValue & _pp;
  const unsigned int _window;
  const MooseEnum _statistic;
  const Real _start_time;
  const Real _reference;

  /// Restartable state
  std::vector<Real> & _history;
  int & _last_step;
  bool & _averaging;
  Real & _sum;
  Real & _avg_time;

  Real _value;
};

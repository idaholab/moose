//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

#include <vector>

/**
 * Time statistics of another postprocessor, using only samples with t >= start_time.
 *
 *   mean                 : arithmetic mean over the last `window` steps
 *   std                  : sample standard deviation over the last `window` steps
 *   time_integrated_mean : time-weighted running mean since start_time
 *   statistics_time      : length of the time interval of time_integrated_mean
 *
 * Before start_time: mean and time_integrated_mean return the instantaneous value,
 * std and statistics_time return 0.
 *
 * Averaging starts at the first time step with t >= start_time. The part of that time step
 * before start_time is excluded from the time integral. One sample is recorded per time step,
 * so use the default execute_on = TIMESTEP_END. All history is restartable.
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
  /// Statistics that can be reported, see the class description
  CreateMooseEnumClass(Statistic, MEAN, STD, TIME_INTEGRATED_MEAN, STATISTICS_TIME);

  /**
   * Arithmetic mean of the stored samples in the half-open index range [begin, end)
   * @param begin index of the first sample in _history to include
   * @param end one past the index of the last sample to include
   * @return the mean, or 0 if the range is empty
   */
  Real windowMean(std::size_t begin, std::size_t end) const;

  /// Value of the monitored postprocessor
  const PostprocessorValue & _pp;
  /// Number of most recent time steps used by the windowed statistics (mean, std)
  const unsigned int _window;
  /// Statistic reported by getValue()
  const Statistic _statistic;
  /// Simulation time from which samples are recorded
  const Real _start_time;
  /// Normalization scale for a relative statistic; 0 means use the window mean magnitude
  const Real _reference;

  // Restartable state

  /// Recorded samples, one per time step, most recent last; capped at 2 * _window entries
  std::vector<Real> & _history;
  /// Time step index of the last recorded sample, used to record at most one sample per step
  int & _last_step;
  /// Whether averaging has started, i.e. a time step with t >= start_time has been reached
  bool & _averaging;
  /// Time integral of the monitored value since start_time, with the rectangle rule
  Real & _sum;
  /// Length of the time integration interval, the sum of the time steps clipped to start_time
  Real & _avg_time;

  /// Value of the selected statistic computed in the latest execute()
  Real _value;
};

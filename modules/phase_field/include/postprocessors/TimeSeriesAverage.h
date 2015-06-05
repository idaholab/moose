/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef TIMESERIESAVERAGE_H
#define TIMESERIESAVERAGE_H

#include "GeneralPostprocessor.h"

// Forward Declarations
class TimeSeriesAverage;

template <>
InputParameters validParams<TimeSeriesAverage>();

class TimeSeriesAverage : public GeneralPostprocessor
{
public:
  TimeSeriesAverage(const InputParameters & parameters);

  virtual void initialize() {}
  virtual void execute();

  /// This will return the running average over the last PostProcessor values
  virtual Real getValue();

protected:
  Real stepAverage() const;
  Real timeAverage() const;

  /// we need this to access the time step history
  FEProblemBase & _feproblem;

  /// slave postprocessor value
  const Real & _slave_pp;

  /// average over time or steps
  enum class AverageMode
  {
    STEPS,
    TIME
  } _average_mode;

  /// either go back a fixed number of steps...
  unsigned int _previous_steps;

  /// ...or go back a set amount of time
  Real _time_window;

  /// History data
  struct HistoryItem
  {
    Real _dt;
    Real _value;
  };

  /// History of time step sizes and slave postprocessor values
  std::vector<HistoryItem> _history;
};

#endif // TIMESERIESAVERAGE_H

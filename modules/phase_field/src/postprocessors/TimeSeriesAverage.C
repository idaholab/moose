/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "TimeSeriesAverage.h"
#include "FEProblem.h"

template <>
InputParameters
validParams<TimeSeriesAverage>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<PostprocessorName>("average", "The postprocessor to average");
  params.addParam<unsigned int>("steps", "Number of previous steps to average over");
  params.addParam<Real>("time", "Time interval to average over");
  return params;
}

TimeSeriesAverage::TimeSeriesAverage(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _feproblem(dynamic_cast<FEProblemBase &>(_subproblem)),
    _slave_pp(getPostprocessorValue("average")),
    _average_mode(isParamValid("steps") ? AverageMode::STEPS : AverageMode::TIME),
    _previous_steps(isParamValid("steps") ? getParam<unsigned int>("steps") : 0),
    _time_window(isParamValid("time") ? getParam<Real>("time") : 0.0),
    _history(0)
{
  if (isParamValid("steps") == isParamValid("time"))
    mooseError("Specify exactly one of the 'steps' and 'time' parameters");
}

void
TimeSeriesAverage::execute()
{
  // push current state into history
  HistoryItem current;
  current._dt = _feproblem.dt();
  current._value = _slave_pp;
  _history.push_back(current);
}

Real
TimeSeriesAverage::getValue()
{
  // At the first time step we return 0.0, otherwise perform desired average
  if (_history.size() == 0)
    return 0.0;

  switch (_average_mode)
  {
    case AverageMode::STEPS:
      return stepAverage();

    case AverageMode::TIME:
      return timeAverage();
  }

  mooseError("Invalid average mode.");
}

Real
TimeSeriesAverage::stepAverage() const
{
  int i = _history.size();
  int back = i - _previous_steps;
  unsigned int n = 0;
  Real sum = 0.0;

  while (i > back && i > 0)
  {
    const HistoryItem & h = _history[--i];
    sum += h._value;
    ++n;
  }

  mooseAssert(n > 0, "Cannot average over empty history.");
  return sum / Real(n);
}

Real
TimeSeriesAverage::timeAverage() const
{
  int i = _history.size();
  Real sum = 0.0;
  Real t = 0.0;

  while (t < _time_window && i > 0)
  {
    const HistoryItem & h = _history[--i];
    t += h._dt;
    sum += h._value * (t < _time_window ? h._dt : h._dt - (t - _time_window));
  }

  const Real averaged_time = std::min(t, _time_window);
  return averaged_time > 0.0 ? sum / averaged_time : 0.0;
}

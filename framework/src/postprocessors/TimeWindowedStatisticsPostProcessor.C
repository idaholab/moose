#include "TimeWindowedStatisticsPostProcessor.h"

#include <algorithm>
#include <cmath>

registerMooseObject("MooseApp", TimeWindowedStatisticsPostProcessor);

InputParameters
TimeWindowedStatisticsPostProcessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription(
      "Windowed statistics (mean, std, drift) and a running time average of a postprocessor, "
      "using only samples from start_time onwards.");

  params.addRequiredParam<PostprocessorName>("postprocessor", "The postprocessor to monitor.");
  params.addRequiredParam<Real>("start_time",
                                "Simulation time from which samples are used for all statistics.");
  params.addRangeCheckedParam<unsigned int>(
      "window", 100, "window > 1", "Number of time steps in one window.");

  MooseEnum stats("mean std cumulative_mean averaging_time", "mean");
  params.addParam<MooseEnum>("statistic", stats, "Which statistic to report.");

  params.addParam<Real>("reference_value",
                        0.0,
                        "Scale used to normalize relative_drift. "
                        "If 0, the magnitude of the window mean is used.");
  return params;
}

TimeWindowedStatisticsPostProcessor::TimeWindowedStatisticsPostProcessor(
    const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _pp(getPostprocessorValue("postprocessor")),
    _window(getParam<unsigned int>("window")),
    _statistic(getParam<MooseEnum>("statistic")),
    _start_time(getParam<Real>("start_time")),
    _reference(getParam<Real>("reference_value")),
    _history(declareRestartableData<std::vector<Real>>("history")),
    _last_step(declareRestartableData<int>("last_step", -1)),
    _averaging(declareRestartableData<bool>("averaging", false)),
    _sum(declareRestartableData<Real>("sum", 0.0)),
    _avg_time(declareRestartableData<Real>("avg_time", 0.0)),
    _value(0.0)
{
}

Real
TimeWindowedStatisticsPostProcessor::windowMean(std::size_t begin, std::size_t end) const
{
  Real s = 0.0;
  for (std::size_t i = begin; i < end; ++i)
    s += _history[i];
  return end > begin ? s / (end - begin) : 0.0;
}

void
TimeWindowedStatisticsPostProcessor::execute()
{
  if (!_averaging && _t >= _start_time)
    _averaging = true;

  // Before start_time: no statistics yet
  if (!_averaging)
  {
    if (_statistic == "mean" || _statistic == "cumulative_mean")
      _value = _pp;
    else // std, averaging_time
      _value = 0.0;
    return;
  }

  // Record at most one sample per time step (guards against repeated execution)
  if (_t_step != _last_step)
  {
    _last_step = _t_step;

    _history.push_back(_pp);
    if (_history.size() > 2 * _window)
      _history.erase(_history.begin());

    _sum += _pp * _dt;
    _avg_time += _dt;
  }

  const std::size_t n = _history.size();
  const std::size_t nw = std::min<std::size_t>(n, _window);

  if (_statistic == "mean")
    _value = windowMean(n - nw, n);

  else if (_statistic == "std")
  {
    const Real m = windowMean(n - nw, n);
    Real s = 0.0;
    for (std::size_t i = n - nw; i < n; ++i)
      s += (_history[i] - m) * (_history[i] - m);
    _value = nw > 1 ? std::sqrt(s / (nw - 1)) : 0.0;
  }

  else if (_statistic == "cumulative_mean")
    _value = _sum / _avg_time;

  else // averaging_time
    _value = _avg_time;
}

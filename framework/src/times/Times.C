//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Times.h"
#include "libmesh/parallel_algebra.h"

InputParameters
Times::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addParam<bool>(
      "unique_times", true, "Whether duplicate values should be removed from the Times vector");
  params.addParam<Real>(
      "unique_tolerance",
      1e-12,
      "Absolute tolerance for removing duplicated times when getting a set of unique times");
  params.addParam<bool>("auto_sort", true, "Whether Times should be sorted");
  // This parameter should be set by each derived class depending on whether the generation of
  // times is replicated or distributed. We want times to be replicated across all ranks
  params.addRequiredParam<bool>("auto_broadcast",
                                "Wether Times should be broadcasted across all ranks");
  params.addParamNamesToGroup("auto_broadcast auto_sort unique_times unique_tolerance", "Advanced");

  // Unlikely to ever be used as Times do not loop over the mesh and use material properties,
  // let alone stateful
  params.suppressParameter<MaterialPropertyName>("prop_getter_suffix");
  params.suppressParameter<bool>("use_interpolated_state");

  params.addParam<bool>("dynamic_time_sequence",
                        true,
                        "Whether the time sequence is dynamic and thus needs to be updated");

  params.registerBase("Times");
  return params;
}

Times::Times(const InputParameters & parameters)
  : GeneralReporter(parameters),
    // Times will be replicated on every rank
    _times(declareValueByName<std::vector<Real>, ReporterVectorContext<Real>>(
        "times", REPORTER_MODE_REPLICATED)),
    _need_broadcast(getParam<bool>("auto_broadcast")),
    _need_sort(getParam<bool>("auto_sort")),
    _need_unique(getParam<bool>("unique_times")),
    _unique_tol(getParam<Real>("unique_tolerance")),
    _dynamic_time_sequence(getParam<bool>("dynamic_time_sequence"))
{
}

Real
Times::getTimeAtIndex(unsigned int index) const
{
  if (_times.size() < index)
    mooseError("Times retrieved with an out-of-bound index");

  if (_times.size())
    return _times[index];
  else
    mooseError("Times vector has not been initialized.");
}

Real
Times::getPreviousTime(const Real current_time) const
{
  if (!_times.size())
    mooseError("Times vector has not been initialized.");

  Real previous_time = _times[0];
  for (const auto i : make_range(std::size_t(1), _times.size()))
  {
    const auto & time = _times[i];
    if (MooseUtils::absoluteFuzzyGreaterThan(time, current_time))
      return previous_time;
    previous_time = time;
  }
  // entire Times vector is prior to current_time
  return previous_time;
}

Real
Times::getNextTime(const Real current_time, const bool error_if_no_next) const
{
  for (const auto i : index_range(_times))
  {
    const auto & time = _times[i];
    if (MooseUtils::absoluteFuzzyGreaterThan(time, current_time))
      return time;
  }
  if (_times.size() && error_if_no_next)
    mooseError("No next time in Times vector for time ",
               current_time,
               ". Maximum time in vector is ",
               *_times.rbegin());
  else if (!error_if_no_next)
    return std::numeric_limits<Real>::max();
  else
    mooseError("Times vector has not been initialized.");
}

const std::vector<Real> &
Times::getTimes() const
{
  if (_times.size())
    return _times;
  else
  {
    // If times array is empty, use current time as start time in the time array
    _times.push_back(getCurrentTime());
    return _times;
  }
}

void
Times::clearTimes()
{
  _times.clear();
}

void
Times::finalize()
{
  if (_need_broadcast)
    // The consumer/producer reporter interface can keep track of whether a reduction is needed
    // (for example if a consumer needs replicated data, but the producer is distributed) however,
    // we have currently made the decision that times should ALWAYS be replicated
    _communicator.allgather(_times, /* identical buffer lengths = */ false);

  if (_need_sort)
    std::sort(_times.begin(), _times.end());
  if (_need_unique)
    _times.erase(unique(_times.begin(),
                        _times.end(),
                        [this](Real l, Real r) { return std::abs(l - r) < _unique_tol; }),
                 _times.end());
}

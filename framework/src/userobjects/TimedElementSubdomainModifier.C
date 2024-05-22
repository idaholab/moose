//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Base class to move elements to a specific subdomain at the given times.

#include "TimedElementSubdomainModifier.h"

registerMooseObject("MooseApp", TimedElementSubdomainModifier);

InputParameters
TimedElementSubdomainModifier::validParams()
{
  InputParameters params = ElementSubdomainModifier::validParams();
  params.addParam<std::vector<Real>>("times", "The times of the subdomain modifications.");

  return params;
}

TimedElementSubdomainModifier::TimedElementSubdomainModifier(const InputParameters & parameters)
  : ElementSubdomainModifier(parameters)
{
}

void
TimedElementSubdomainModifier::initialize()
{
  // state variables
  _current_timespan_start = -1;
  _current_step = std::numeric_limits<unsigned int>::max();

  // ask for all times (must NOT be sorted)
  const auto _times = onGetTimes();
  const auto n = _times.size();

  // copy data to local storage
  _times_and_indices.resize(n);
  for (auto i : make_range(n))
  {
    _times_and_indices[i].time = _times[i];
    _times_and_indices[i].index = i;
  };

  // sort the times
  std::sort(_times_and_indices.begin(), _times_and_indices.end());
}

std::vector<Real>
TimedElementSubdomainModifier::onGetTimes()
{
  const auto n = _times_and_indices.size();
  std::vector<Real> _times;
  _times.resize(n);
  for (auto i : make_range(n))
  {
    _times[i] = _times_and_indices[i].time;
  };
  return _times;
}

SubdomainID
TimedElementSubdomainModifier::onComputeSubdomainID(Real /* t_from_exclusive */,
                                                    Real /* t_to_inclusive */)
{
  const auto _subdomain_id = _current_elem->subdomain_id();
  return _subdomain_id;
}

SubdomainID
TimedElementSubdomainModifier::computeSubdomainID()
{
  // did we advance to the next step?
  if (_current_step != unsigned(_t_step))
  {
    _current_timespan_start = _current_timespan_end;
    _current_timespan_end = _t;
    _current_step = _t_step;
  }

  // get the new subdomain-id of the current element; provide the timespan to be considered
  const auto _subdomain_id = onComputeSubdomainID(_current_timespan_start, _current_timespan_end);

  return _subdomain_id;
}

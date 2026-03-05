//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Times.h"

#include "MooseError.h"
#include "MooseUtils.h"

Times::Times(std::vector<Real> & times, const Real & current_time, bool is_dynamic)
  : _times(times), _current_time(current_time), _dynamic_time_sequence(is_dynamic)
{
}

Real
Times::getTimeAtIndex(unsigned int index) const
{
  if (_times.size() < index)
    ::mooseError("Times retrieved with an out-of-bound index");

  if (_times.size())
    return _times[index];
  else
    ::mooseError("Times vector has not been initialized.");
}

Real
Times::getPreviousTime(const Real current_time, bool error_if_no_previous) const
{
  if (_times.empty())
    ::mooseError("Times vector has not been initialized.");
  mooseAssert(std::is_sorted(_times.begin(), _times.end()), "Times vector must be sorted.");

  // If all times are greater than current_time
  if (_times.front() > current_time)
  {
    if (error_if_no_previous)
      ::mooseError("No previous time in Times vector for time ",
                   current_time,
                   ". Minimum time in vector is ",
                   _times.front());
    return -std::numeric_limits<Real>::max();
  }

  for (const auto i : make_range(std::size_t(1), _times.size()))
    if (MooseUtils::absoluteFuzzyGreaterThan(_times[i], current_time))
      return _times[i - 1];
  // entire Times vector is prior to current_time
  return _times.back();
}

Real
Times::getNextTime(const Real current_time, const bool error_if_no_next) const
{
  if (_times.empty())
    ::mooseError("Times vector has not been initialized.");
  mooseAssert(std::is_sorted(_times.begin(), _times.end()), "Times vector must be sorted.");

  auto it = std::find_if(_times.begin(),
                         _times.end(),
                         [&](Real time)
                         { return MooseUtils::absoluteFuzzyGreaterThan(time, current_time); });

  // entire Times vector is prior to current_time
  if (it == _times.end())
  {
    if (error_if_no_next)
      ::mooseError("No next time in Times vector for time ",
                   current_time,
                   ". Maximum time in vector is ",
                   *_times.rbegin());
    return std::numeric_limits<Real>::max();
  }

  return *it;
}

const std::vector<Real> &
Times::getTimes() const
{
  if (_times.size())
    return _times;
  else
    ::mooseError("Times vector has not been initialized.");
}

void
Times::clearTimes()
{
  _times.clear();
}

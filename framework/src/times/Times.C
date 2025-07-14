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

Times::Times(std::vector<Real> & times, const Real & current_time)
  : _times(times), _current_time(current_time)
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
Times::getPreviousTime(const Real current_time) const
{
  if (!_times.size())
    ::mooseError("Times vector has not been initialized.");

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
    ::mooseError("No next time in Times vector for time ",
                 current_time,
                 ". Maximum time in vector is ",
                 *_times.rbegin());
  else if (!error_if_no_next)
    return std::numeric_limits<Real>::max();
  else
    ::mooseError("Times vector has not been initialized.");
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

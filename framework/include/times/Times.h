//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <vector>
#include <set>

#include "libmesh/libmesh_common.h"

using libMesh::Real;

/**
 * Base Times object that implements public interfaces. Derived objects are
 * meant to pass in a vector of times. See TimesReporter for the user-facing
 * instantiation of this class. The purpose of this abstraction is to allow a
 * "default" Times object that can be constructed without going through the
 * Moose factory.
 */
class Times
{
public:
  Times(std::vector<Real> & times, const Real & current_time, bool is_dynamic);

  /// Getter for the full times vector
  const std::vector<Real> & getTimes() const;

  /// Getter for a set of the full times
  const std::set<Real> getUniqueTimes() const
  {
    return std::set<Real>(_times.begin(), _times.end());
  };

  /// Get the current time
  Real getCurrentTime() const { return _current_time; }

  /// Getter for a single time at a known index
  Real getTimeAtIndex(unsigned int index) const;

  /// Find the previous time in the times vector for a given time
  /// If current_time is also in the times vector within numerical precision, will return the previous value
  /// If the times are not sorted or unique, this will return the time right before the first time found above the current_time
  Real getPreviousTime(const Real current_time, bool error_if_no_previous = true) const;

  /// Find the next time in the times vector for a given time
  /// If current_time is also in the times vector within numerical precision, will return the next value
  /// @param current_time the time we want the next time for
  /// @param error_if_no_next whether to error if the current time is beyond all existing times in the vector
  ///                         or return instead the largest Real number (from std::numeric_limits)
  Real getNextTime(const Real current_time, const bool error_if_no_next) const;

  /// Whether the sequence of times might change during the simulation
  bool isDynamicTimeSequence() const { return _dynamic_time_sequence; };

protected:
  /// Clear the times vector
  void clearTimes();

  /// A reference to the passed in vector of times
  std::vector<Real> & _times;

private:
  /// A reference to current time, used by getCurrentTime()
  const Real & _current_time;

  /// whether the time sequence is set dynamically
  const bool _dynamic_time_sequence;
};

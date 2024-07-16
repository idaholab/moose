//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose includes
#include "GeneralReporter.h"

/**
 * Times objects are under the hood Reporters, but limited to a vector of Real
 */
class Times : public GeneralReporter
{
public:
  static InputParameters validParams();
  Times(const InputParameters & parameters);
  virtual ~Times() = default;

  /// Getter for the full times vector
  const std::vector<Real> & getTimes() const;

  /// Getter for a set of the full times
  const std::set<Real> getUniqueTimes() const
  {
    return std::set<Real>(_times.begin(), _times.end());
  };

  /// Get the current time
  Real getCurrentTime() const { return _fe_problem.time(); }

  /// Getter for a single time at a known index
  Real getTimeAtIndex(unsigned int index) const;

  /// Find the previous time in the times vector for a given time
  /// If current_time is also in the times vector within numerical precision, will return the previous value
  /// If the times are not sorted or unique, this will return the time right before the first time found above the current_time
  Real getPreviousTime(const Real current_time) const;

  /// Find the next time in the times vector for a given time
  /// If current_time is also in the times vector within numerical precision, will return the next value
  /// @param current_time the time we want the next time for
  /// @param error_if_no_next whether to error if the current time is beyond all existing times in the vector
  ///                         or return instead the largest Real number (from std::numeric_limits)
  Real getNextTime(const Real current_time, const bool error_if_no_next) const;

protected:
  /// In charge of computing / loading the times, unless all that could be done there is done
  /// in the constructor
  virtual void initialize() override = 0;

  /// By default, we wont execute often but "executing" will mean loading the times
  virtual void execute() override { initialize(); }

  /// In charge of reduction across all ranks
  virtual void finalize() override;

  /// By default, Times will not be modified very regularly
  virtual void timestepSetup() override {}
  virtual void residualSetup() override {}
  virtual void jacobianSetup() override {}

  /// Clear the times vector
  void clearTimes();

  /// The vector holding the times
  std::vector<Real> & _times;

  /// Whether generation of times is distributed or not (and therefore needs a broadcast)
  const bool _need_broadcast;
  /// Whether times should be sorted, because they come from different sources for example
  const bool _need_sort;
  /// Whether duplicate times should be removed
  const bool _need_unique;
  /// Absolute tolerance for performing duplication checks to make the times vector unique
  const Real _unique_tol;
};

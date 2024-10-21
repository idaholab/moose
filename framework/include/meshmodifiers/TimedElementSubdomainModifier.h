//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementSubdomainModifier.h"

/**
 * Modifies element subdomains only at a given list of times
 */
class TimedElementSubdomainModifier : public ElementSubdomainModifier
{
public:
  static InputParameters validParams();

  TimedElementSubdomainModifier(const InputParameters & parameters);

  virtual void initialize() override;

protected:
  /**
   * Requests a vector of all times from the inheriting class
   * (these do not have to be sorted and may have duplicates).
   * @returns Unsorted vector of times.
   */
  virtual std::vector<Real> getTimes() = 0;

  /// storage for the times including their original index.
  struct TimeIndexPair
  {
    Real time;
    std::size_t index;

    bool operator<(const TimeIndexPair & a) const
    {
      if (time == a.time)
        return index < a.index;
      else
        return time < a.time;
    }
  };

  /// Times and subdomain changes to make
  std::set<TimeIndexPair> _times_and_indices;
};

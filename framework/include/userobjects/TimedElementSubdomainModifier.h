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
  virtual SubdomainID computeSubdomainID() override;

  /**
   * Requests a vector of all points in time from the inheriting class
   * (these do not have to be sorted).
   * @returns Unsorted vector of points in time.
   */
  virtual std::vector<Real> onGetTimes();

  /**
   * Determines the new subdomain-id for the current element.
   * @returns New subdomain id to move the element to.
   */
  virtual SubdomainID onComputeSubdomainID(Real t_from_exclusive, Real t_to_inclusive);

  /// storage for the times including their original index.
  struct timeIndexPair
  {
    Real time;
    std::size_t index;

    bool operator<(const timeIndexPair & a) const
    {
      if (time == a.time)
        return index < a.index;
      else
        return time < a.time;
    }
  };

  /// Times and subdomain changes to make
  std::vector<timeIndexPair> _times_and_indices;

private:
  /// Local variable for the current step (to be able to determine incrementation)
  unsigned int _current_step;

  /// Local variable for the end of the timespan to apply changes of the subdomain
  Real _current_timespan_end;

  /// Local variable for the start of the timespan to apply changes of the subdomain
  Real _current_timespan_start;
};

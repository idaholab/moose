//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/utility.h"

#include <map>
#include <string>

/**
 * Storage for the number of times an object has received each setup method.
 *
 * This is deliberately not a template so that FVSetupCount has a concrete type to cross-cast to;
 * FVSetupTester is a template and so cannot serve that purpose itself.
 */
class FVSetupCounter
{
public:
  virtual ~FVSetupCounter() = default;

  /// The number of times the setup method named \p type has been called on this object. Keys
  /// that were never incremented count zero, so that a per-flag key for an execution flag this
  /// run never issued reads as zero rather than erroring
  unsigned int getSetupCount(const std::string & type) const
  {
    const auto it = _counts.find(type);
    return it == _counts.end() ? 0 : it->second;
  }

protected:
  /// Record one call of the setup method named \p type
  void incrementSetupCount(const std::string & type) { _counts[type]++; }

private:
  /// Counts keyed by the count_type names that FVSetupCount accepts, and additionally by
  /// "CUSTOM_<flag>" for each execution flag customSetup has been called with
  std::map<std::string, unsigned int> _counts = {{"INITIAL", 0}, {"TIMESTEP", 0}, {"CUSTOM", 0}};
};

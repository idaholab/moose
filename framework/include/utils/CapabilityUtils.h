//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <variant>
#include <string>
#include <map>

/**
 * Shared code for the Capabilities Registry and the python bindings to teh Capabilities system.
 */
namespace CapabilityUtils
{

/// A capability can have a bool, int, or string value
typedef std::variant<bool, int, std::string> Type;

typedef std::map<std::string, std::pair<CapabilityType, std::string>> Registry;

/// Check return status
enum class Status
{
  PASS = 0,    // Requirement is fulfilled
  UNKNOWN = 1, // An unregistred requirement is encountered
  FAIL = 2     // A requirement is explicitly violated
};

/// Check a requirement against a capabilities registry
std::pair<Status, std::string> Status check(const std::string & requirement,
                                            const Registry & capabilities);

} // namespace CapabilityUtils

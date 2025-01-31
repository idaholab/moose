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
 * Shared code for the Capabilities Registry and the python bindings to the Capabilities system.
 */
namespace CapabilityUtils
{

/**
 * Return state for check. We use a plain enum because we rely on implicit conversion to int.
 * Capability checks are run in the test harness using the JSON dump exported from the executable
 * using `--show-capabilities`. This static check does not take dynamic loading into account,
 * so some capabilites might not be registered at all. Whenever a test agains an unregistered
 * capability is made the result is returned as POSSIBLE_FAIL/POSSIBLE_PASS (in case of predicate
 * negation).
 * The test harness will run the test unless the result of the check is CERTAIN_FAIL. The runtime
 * check in the executable will terminate if the result is either CERTAIN_FAIL or POSSIBLE_FAIL.
 */
enum CheckState
{
  CERTAIN_FAIL = 0,
  POSSIBLE_FAIL = 1,
  UNKNOWN = 2,
  POSSIBLE_PASS = 3,
  CERTAIN_PASS = 4,
  PARSE_FAIL = 5
};

/// A capability can have a bool, int, or string value
typedef std::variant<bool, int, std::string> Type;
typedef std::tuple<CheckState, std::string, std::string> Result;
typedef std::map<std::string, std::pair<Type, std::string>> Registry;

/// Check a requirement against a capabilities registry
Result check(std::string requirements, const Registry & capabilities);

} // namespace CapabilityUtils

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
 * as capabilities that would be registered after initializing the dynamically loaded application
 * will not exist with `--show-capabilities`.
 *
 * A requested capability that is not registered at all is considered in a "possible" state,
 * as we cannot guarantee that it does or not exist with a dynamic application. If no dynamic
 * application loading is used, the possible states can be considered certain states.
 *
 * When the test harness Tester specification "dynamic_capabilities" is set to True,
 * it will run the test unless the result of the check is CERTAIN_FAIL. In this case,
 * the runtime check in the executable will terminate if the result is either CERTAIN_FAIL
 * or POSSIBLE_FAIL.
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
/// Result from a capability check: the state, the reason, and the documentation
typedef std::tuple<CheckState, std::string, std::string> Result;
/// The registry that stores the registered capabilities
typedef std::map<std::string, std::pair<Type, std::string>> Registry;

/**
 * Checks if a set of requirements is satisified by the given capability registry
 *
 * @param requirements The requirement string
 * @param capabilities The registry that contains the capabilities
 *
 * This method is exposed to Python within the capabilities_check method in
 * framework/contrib/capabilities/capabilities.C. This external method is used
 * significantly by the TestHarness to check capabilities for individual test specs.
 *
 * Additionally, this method is used by the MooseApp command line option
 * "--required-capabilities ...".
 *
 * Requirements can use comparison operators (>,<,>=,<=,=!,=), where the name of
 * the capability must always be on the left hand side. Comparisons can be performed
 * on strings "compiler!=GCC" (which are case insensitive), integer numbers
 * "ad_size>=50", and version numbers "petsc>3.8.0". The state of a boolean
 * valued capability can be tested by just specifying the capability name "chaco".
 * This check can be inverted using the ! operator as "!chaco".
 *
 * The logic operators & and | can be used to chain multiple checks as
 * "thermochimica & thermochimica>1.0". Parenthesis can be used to build
 * complex logic expressions.
 *
 * See the description for CheckState for more information on why a
 * certain state would be returned.
 */
Result check(std::string requirements, const Registry & capabilities);

} // namespace CapabilityUtils

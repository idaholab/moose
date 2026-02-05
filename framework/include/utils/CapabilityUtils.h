//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "StreamArguments.h"

#include <exception>
#include <variant>
#include <string>
#include <sstream>
#include <tuple>
#include <map>
#include <vector>
#include <optional>
#include <string_view>

#ifdef MOOSE_UNIT_TEST
// forward declare unit tests
#include "gtest/gtest.h"
class GTEST_TEST_CLASS_NAME_(CapabilityTest, capabilityNegateValue);
class CapabilitiesTest;
#endif

/**
 * Shared code for the Capabilities Registry and the python bindings to the Capabilities system.
 */
namespace Moose::CapabilityUtils
{

class CapabilityException : public std::runtime_error
{
public:
  CapabilityException(const CapabilityException &) = default;

  template <typename... Args>
  static std::string stringify(Args &&... args)
  {
    std::ostringstream ss;
    streamArguments(ss, args...);
    return ss.str();
  }

  template <typename... Args>
  explicit CapabilityException(Args &&... args) : std::runtime_error(stringify(args...))
  {
  }

  ~CapabilityException() throw() {}
};

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
  CERTAIN_PASS = 4
};

/// A capability can have a bool, int, or string value
typedef std::variant<bool, int, std::string> CapabilityValue;

/**
 * An entry for a single capability.
 */
class Capability
{
public:
  Capability() = delete;
  Capability(const std::string_view name,
             const CapabilityValue & value,
             const std::string_view doc);

  /**
   * @return The name of the capability.
   */
  const std::string & getName() const { return _name; }

  /**
   * @return The documentation string.
   */
  const std::string & getDoc() const { return _doc; }

  /**
   * @return The capability value.
   */
  const CapabilityValue & getValue() const { return _value; }

  /**
   * @return Whether or not the capability is explicit.
   *
   * Explicit implies that the capability cannot be compared as a boolean.
   */
  bool getExplicit() const { return _explicit; }

  /**
   * @return The enumeration, if set.
   *
   * This is only valid for string valued capabilities.
   */
  const std::optional<std::vector<std::string>> & getEnumeration() const;

  /**
   * @return Whether or not the capability has the given enumeration \p value.
   *
   * This is only valid for string-valued capabilities.
   *
   * If the capability has no enumerations, this will always return true.
   */
  bool hasEnumeration(const std::string & value) const;

  /**
   * Set the capability to be explicit.
   *
   * Explicit implies that the capability cannot be compared as a boolean.
   *
   * This is only valid for non-bool valued capabilities.
   */
  Capability & setExplicit();

  /**
   * Set the enumeration (allowed values) for the capability.
   *
   * This is only valid for string-valued capabilities.
   */
  Capability & setEnumeration(const std::vector<std::string> & enumeration);

#if defined(MOOSE_UNIT_TEST) || defined(FOR_PYCAPABILITIES)
  /**
   * Negate a Capability value.
   *
   * This should only be used by pycapabilities via the
   * TestHarness when it needs to augment capabilities
   * to check on if a check depeneds on a capability or not.
   */
  inline void negateValue();
#endif

  /**
   * @return The boolean capability value if it is a boolean.
   */
  const bool * queryBoolValue() const;
  /**
   * @return The integer capability value if it is an integer.
   */
  const int * queryIntValue() const;
  /**
   * @return The string capability value if it is a string.
   */
  const std::string * queryStringValue() const;

  /**
   * @return Whether or not the capability value is a boolean.
   */
  bool hasBoolValue() const { return queryBoolValue(); }

  /**
   * @return Whether or not the capability value is an integer.
   */
  bool hasIntValue() const { return queryIntValue(); }

  /**
   * @return Whether or not the capability value is a string.
   */
  bool hasStringValue() const { return queryStringValue(); }

  /**
   * @return The capability value as a string.
   */
  std::string valueToString() const;

  /**
   * @return The capability as a string in the form of "name=value".
   */
  std::string toString() const;

  /**
   * @return The enumeration as a string of comma separated values.
   *
   * This is only valid for a capability that has an enumeration.
   */
  std::string enumerationToString() const;

private:
#ifdef MOOSE_UNIT_TEST
  FRIEND_TEST(::CapabilityTest, capabilityNegateValue);
#endif

  /// The name of capability
  std::string _name;
  /// Description for the capability
  std::string _doc;
  /// The value the capability is set to
  CapabilityValue _value;
  /// Whether or not this capability must be compared explicitly
  /// (not as a boolean check)
  bool _explicit;
  /// Possible enumeration for the capability, if any (string capabilities only)
  std::optional<std::vector<std::string>> _enumeration;
};

/**
 * Registry of capabilities that checks capability requirements.
 *
 * This registry is used both within MOOSE (in framework/src/base/Capabilities.C)
 * and within the python interface (in python/pycapabilities/_pycapabilities.C).
 */
class CapabilityRegistry
{
public:
  /// Result from a capability check: the state, the reason, and the documentation
  using Result = std::tuple<CheckState, std::string, std::string>;

  /// Type for the registry
  using RegistryType = std::map<std::string, Capability, std::less<>>;

  /**
   * Add a capability.
   *
   * @param registry The registry
   * @param capability The name of the capability
   * @param value The value of the capability
   * @param doc The documentation string
   * @return The capability
   */
  Capability & add(const std::string_view name,
                   const CapabilityUtils::CapabilityValue & value,
                   const std::string_view doc);

  /**
   * Query a Capability.
   *
   * Will convert the capability name to lowercase.
   */
  ///@{
  const Capability * query(std::string capability) const;
  Capability * query(std::string capability);
  ///@}

  /**
   * Get a capability.
   *
   * Will not convert the capability name to lowercase.
   */
  Capability & get(const std::string_view capability);

  /**
   * Get the underlying registry.
   */
  const RegistryType & getRegistry() const { return _registry; }

  /**
   * Checks if a set of requirements is satisified by the capabilities
   *
   * @param capabilities The registry that contains the capabilities
   * @param requirements The requirement string
   *
   * This method is exposed to Python within pycapabilities.Capabilities.check in
   * python/pycapabilities/_pycapabilities.C. This external method is used
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
  Result check(std::string requirements) const;

  /**
   * Clear the registry.
   */
  void clear() { _registry.clear(); }

private:
#ifdef MOOSE_UNIT_TEST
  friend class ::CapabilitiesTest;
#endif

  /// Registry storage
  RegistryType _registry;
};

#if defined(MOOSE_UNIT_TEST) || defined(FOR_PYCAPABILITIES)
void
Capability::negateValue()
{
  _explicit = false;
  _enumeration.reset();
  _value = false;
}
#endif

} // namespace CapabilityUtils

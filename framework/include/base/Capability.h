//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <optional>
#include <set>
#include <string>
#include <variant>

#ifdef MOOSE_UNIT_TEST
// forward declare unit tests
#include "gtest/gtest.h"
class GTEST_TEST_CLASS_NAME_(CapabilityTest, negateValue);
class GTEST_TEST_CLASS_NAME_(CapabilitiesTest, isInstallationType);
#endif

namespace Moose
{
/**
 * An entry for a single capability.
 */
class Capability
{
public:
  /// A capability can have a bool, int, or string value
  using Value = std::variant<bool, int, std::string>;

  Capability() = delete;
  Capability(const std::string_view name,
             const Capability::Value & value,
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
  const Capability::Value & getValue() const { return _value; }

  /**
   * @return Whether or not the capability is explicit.
   *
   * Explicit implies that the capability cannot be compared as a boolean.
   */
  bool getExplicit() const { return _explicit; }

  /**
   * @return The enumeration, if set.
   *
   * Only string-valued capabilites can have an enumeration.
   */
  const std::optional<std::set<std::string>> & queryEnumeration() const { return _enumeration; }

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
  Capability & setEnumeration(const std::set<std::string> & enumeration);

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
  const bool * queryBoolValue() const { return std::get_if<bool>(&_value); }
  /**
   * @return The integer capability value if it is an integer.
   */
  const int * queryIntValue() const { return std::get_if<int>(&_value); }
  /**
   * @return The string capability value if it is a string.
   */
  const std::string * queryStringValue() const { return std::get_if<std::string>(&_value); }

  /**
   * @return Whether or not the capability value is a boolean.
   */
  bool hasBoolValue() const { return std::holds_alternative<bool>(_value); }

  /**
   * @return Whether or not the capability value is an integer.
   */
  bool hasIntValue() const { return std::holds_alternative<int>(_value); }

  /**
   * @return Whether or not the capability value is a string.
   */
  bool hasStringValue() const { return std::holds_alternative<std::string>(_value); }

  /**
   * @return The boolean capability value.
   *
   * Will error if the value is not a boolean.
   */
  bool getBoolValue() const;
  /**
   * @return The boolean capability value.
   *
   * Will error if the value is not an integer.
   */
  int getIntValue() const;
  /**
   * @return The string capability value.
   *
   * Will error if the value is not a string.
   */
  const std::string & getStringValue() const;

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
  FRIEND_TEST(::CapabilityTest, negateValue);
  FRIEND_TEST(::CapabilitiesTest, isInstallationType);
#endif

  /// The name of capability
  std::string _name;
  /// Description for the capability
  std::string _doc;
  /// The value the capability is set to
  Capability::Value _value;
  /// Whether or not this capability must be compared explicitly
  /// (not as a boolean check)
  bool _explicit;
  /// Possible enumeration for the capability, if any (string capabilities only)
  std::optional<std::set<std::string>> _enumeration;
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
}

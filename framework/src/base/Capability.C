//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Capability.h"
#include "CapabilityException.h"

#include "MooseStringUtils.h"

#include <regex>

namespace Moose
{
Capability::Capability(const std::string_view name,
                       const Capability::Value & value,
                       const std::string_view doc)
  : _name(name), _doc(doc), _value(value), _explicit(false)
{
  // Check name validity
  if (getName().empty())
    throw CapabilityException("Capability has empty name");
  if (!std::regex_match(getName(), std::regex("[a-z0-9_-]+")))
    throw CapabilityException(
        "Capability '" + getName() +
        "': Name has unallowed characters; allowed characters = 'a-z, 0-9, _, -'");

  // String value validity
  if (const auto string_ptr = queryStringValue())
  {
    const std::regex string_regex("[a-z0-9_.-]+");
    if (!std::regex_match(*string_ptr, string_regex))
      throw CapabilityException(
          "String capability '" + getName() + "': value '" + *string_ptr +
          "' has unallowed characters; allowed characters = 'a-z, 0-9, _, ., -'");
  }
}

bool
Capability::hasEnumeration(const std::string & value) const
{
  return _enumeration ? _enumeration->count(value) : true;
}

const std::set<std::string> &
Capability::getEnumeration() const
{
  if (!_enumeration)
    throw CapabilityException("Capability::getEnumeration(): Capability '" + getName() +
                              "' does not have an enumeration");
  return *_enumeration;
}

Capability &
Capability::setExplicit()
{
  if (hasBoolValue())
    throw CapabilityException("Capability::setExplicit(): Capability '" + getName() +
                              "' is bool-valued and cannot be set as explicit");
  _explicit = true;
  return *this;
}

Capability &
Capability::setEnumeration(const std::set<std::string> & enumeration)
{
  static const std::string error_prefix = "Capability::setEnumeration(): ";

  const auto string_ptr = queryStringValue();
  if (!string_ptr)
    throw CapabilityException(error_prefix + "Capability '" + getName() +
                              "' is not string-valued and cannot have an enumeration");

  if (_enumeration)
  {
    if (*_enumeration == enumeration)
      return *this;
    throw CapabilityException(error_prefix + "Capability '" + getName() +
                              "' already has an enumeration set");
  }

  if (enumeration.empty())
    throw CapabilityException(error_prefix + "Enumeration is empty for '" + getName() + "'");

  for (const auto & value : enumeration)
    if (!std::regex_match(value, std::regex("[a-z0-9_-]+")))
      throw CapabilityException(error_prefix + "Enumeration value '" + value +
                                "' for capability '" + getName() + "'" +
                                " has unallowed characters; allowed characters = 'a-z, 0-9, _, -'");

  _enumeration = enumeration;

  if (!hasEnumeration(*string_ptr))
    throw CapabilityException(error_prefix + "Capability " + toString() +
                              " value not within enumeration");

  return *this;
}

bool
Capability::getBoolValue() const
{
  if (const auto bool_ptr = queryBoolValue())
    return *bool_ptr;
  throw CapabilityException("Capability::getBoolValue(): Capability " + toString() +
                            " is not a bool");
}

int
Capability::getIntValue() const
{
  if (const auto int_ptr = queryIntValue())
    return *int_ptr;
  throw CapabilityException("Capability::getIntValue(): Capability " + toString() +
                            " is not an integer");
}

const std::string &
Capability::getStringValue() const
{
  if (const auto string_ptr = queryStringValue())
    return *string_ptr;
  throw CapabilityException("Capability::getStringValue(): Capability " + toString() +
                            " is not a string");
}

std::string
Capability::valueToString() const
{
  if (const auto bool_ptr = queryBoolValue())
    return *bool_ptr ? "true" : "false";
  if (const auto string_ptr = queryStringValue())
    return *string_ptr;
  if (const auto int_ptr = queryIntValue())
    return std::to_string(*int_ptr);
  throw CapabilityException("Capability::valueToString(): Invalid type");
}

std::string
Capability::toString() const
{
  return getName() + "=" + valueToString();
}

std::string
Capability::enumerationToString() const
{
  if (_enumeration)
    return MooseUtils::stringJoin(
        std::vector<std::string>(_enumeration->begin(), _enumeration->end()), ", ");
  throw CapabilityException("Capability::enumerationToString(): Capability '",
                            getName(),
                            "' does not have an enumeration");
}
} // namespace Moose

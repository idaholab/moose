//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "peglib.h"

#include "CapabilityUtils.h"
#include "MooseStringUtils.h"
#include <regex>
#include <utility>

namespace Moose::CapabilityUtils
{

const std::set<std::string, std::less<>> augmented_capability_names{
    // TestHarness.getCapabilities()
    "hpc",
    "machine",
    "library_mode",
    // TestHarness.testers.RunApp.getAugmentedCapabilities()
    "mpi_procs",
    "num_threads"};

Capability::Capability(const std::string_view name,
                       const CapabilityValue & value,
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

const std::optional<std::vector<std::string>> &
Capability::getEnumeration() const
{
  if (!hasStringValue())
    throw CapabilityException("Capability::getEnumeration(): Capability '",
                              getName(),
                              "' is not string-valued and cannot have an enumeration");
  return _enumeration;
}

bool
Capability::hasEnumeration(const std::string & value) const
{
  const auto & enumeration_ptr = getEnumeration();
  if (!enumeration_ptr)
    return true;
  return std::find(enumeration_ptr->begin(), enumeration_ptr->end(), value) !=
         enumeration_ptr->end();
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
Capability::setEnumeration(const std::vector<std::string> & enumeration)
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

  for (auto it = enumeration.begin(); it != enumeration.end(); ++it)
    if (std::find(std::next(it), enumeration.end(), *it) != enumeration.end())
      throw CapabilityException(error_prefix + "Duplicate enumeration '" + *it +
                                "' for capability '" + getName() + "'");

  _enumeration = enumeration;

  if (!hasEnumeration(*string_ptr))
    throw CapabilityException(error_prefix + "Capability " + toString() +
                              " value not within enumeration");

  return *this;
}

const bool *
Capability::queryBoolValue() const
{
  return std::get_if<bool>(&_value);
}

const int *
Capability::queryIntValue() const
{
  return std::get_if<int>(&_value);
}

const std::string *
Capability::queryStringValue() const
{
  return std::get_if<std::string>(&_value);
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
  const auto & enumeration_ptr = getEnumeration();
  if (!enumeration_ptr)
    throw CapabilityException("Capability::enumerationToString(): Capability '",
                              getName(),
                              "' does not have an enumeration");
  return MooseUtils::stringJoin(*enumeration_ptr, ", ");
}

Capability &
CapabilityRegistry::add(const std::string_view name,
                        const CapabilityUtils::CapabilityValue & value,
                        const std::string_view doc)
{
  auto it_pair = _registry.lower_bound(name);
  if (it_pair != _registry.end() && it_pair->first == name)
  {
    auto & capability = it_pair->second;
    if (capability.getValue() != value || capability.getDoc() != doc)
      throw CapabilityException("Capability '" + std::string(name) +
                                "' already exists and is not equal");
    return capability;
  }

  return _registry
      .emplace_hint(it_pair,
                    std::piecewise_construct,
                    std::forward_as_tuple(name),
                    std::forward_as_tuple(name, value, doc))
      ->second;
}

const Capability *
CapabilityRegistry::query(std::string capability) const
{
  capability = MooseUtils::toLower(capability);
  if (const auto it = _registry.find(capability); it != _registry.end())
    return &it->second;
  return nullptr;
}

Capability *
CapabilityRegistry::query(std::string capability)
{
  return const_cast<Capability *>(std::as_const(*this).query(capability));
}

const Capability &
CapabilityRegistry::get(const std::string & capability) const
{
  if (const auto capability_ptr = query(capability))
    return *capability_ptr;
  throw CapabilityException("Capability '" + capability + "' not registered");
}

Capability &
CapabilityRegistry::get(const std::string & capability)
{
  return const_cast<Capability &>(std::as_const(*this).get(capability));
}

[[noreturn]] void
checkException(const peg::SemanticValues & vs,
               const std::string & message,
               const std::optional<CapabilityUtils::Capability> capability = {})
{
  std::string msg = "Capability statement '" + vs.token_to_string() + "': ";
  if (capability)
    msg += "capability '" + capability->toString() + "' ";
  msg += message;
  throw CapabilityException(msg);
}

CapabilityRegistry::Result
CapabilityRegistry::check(std::string requirements) const
{
  using namespace peg;

  // unquote
  while (true)
  {
    const auto len = requirements.length();
    if (len >= 2 && ((requirements[0] == '\'' && requirements[len - 1] == '\'') ||
                     (requirements[0] == '"' && requirements[len - 1] == '"')))
      requirements = requirements.substr(1, len - 2);
    else
      break;
  }
  if (requirements.length() == 0)
    return {CapabilityUtils::CERTAIN_PASS, "Empty requirements", ""};

  static parser parser(R"(
    Expression    <-  _ Bool _ LogicOperator _ Expression / Bool _
    Bool          <-  Comparison / '!' Bool / '!' Identifier / Identifier / '(' _ Expression _ ')'
    Comparison    <-  Identifier _ Operator _ Version / Identifier _ Operator _ String
    String        <-  [a-zA-Z0-9_-]+
    Identifier    <-  [a-zA-Z][a-zA-Z0-9_]*
    Operator      <-  [<>=!]+
    LogicOperator <-  [&|]
    Version       <-  Number '.' Version  / Number
    Number        <-  [0-9]+
    ~_            <-  [ \t]*
  )");

  if (!static_cast<bool>(parser))
    throw CapabilityException("Capabilities parser build failure.");

  parser["Number"] = [](const SemanticValues & vs) { return vs.token_to_number<int>(); };

  parser["Version"] = [](const SemanticValues & vs)
  {
    switch (vs.choice())
    {
      case 0: // Number '.' Version
      {
        std::vector<int> ret{std::any_cast<int>(vs[0])};
        const auto & vs1 = std::any_cast<std::vector<int>>(vs[1]);
        ret.insert(ret.end(), vs1.begin(), vs1.end());
        return ret;
      }

      case 1: // Number
        return std::vector<int>{std::any_cast<int>(vs[0])};
    }

    checkException(vs, "unknown number match.");
  };

  enum LogicOperator
  {
    OP_AND,
    OP_OR
  };

  parser["LogicOperator"] = [](const SemanticValues & vs)
  {
    const auto op = vs.token();
    if (op == "&")
      return OP_AND;
    if (op == "|")
      return OP_OR;
    checkException(vs, "unknown logic operator.");
  };

  enum Operator
  {
    OP_LESS_EQ,
    OP_GREATER_EQ,
    OP_LESS,
    OP_GREATER,
    OP_NOT_EQ,
    OP_EQ
  };

  parser["Operator"] = [](const SemanticValues & vs)
  {
    const auto op = vs.token();
    if (op == "<=")
      return OP_LESS_EQ;
    if (op == ">=")
      return OP_GREATER_EQ;
    if (op == "<")
      return OP_LESS;
    if (op == ">")
      return OP_GREATER;
    if (op == "!=")
      return OP_NOT_EQ;
    if (op == "=" || op == "==")
      return OP_EQ;
    checkException(vs, "unknown operator.");
  };

  parser["String"] = [](const SemanticValues & vs) { return vs.token_to_string(); };
  parser["Identifier"] = [](const SemanticValues & vs) { return vs.token_to_string(); };

  parser["Comparison"] = [this](const SemanticValues & vs)
  {
    const auto left = std::any_cast<std::string>(vs[0]);
    const auto op = std::any_cast<Operator>(vs[1]);

    // check existence
    const auto capability_ptr = query(left);
    if (!capability_ptr)
      // return an unknown if the capability does not exist, this is important as it
      // stays unknown upon negation
      return CheckState::UNKNOWN;

    // capability is registered by the app
    const auto & capability = *capability_ptr;

    // explicitly false causes any comparison to fail
    if (const auto bool_ptr = capability.queryBoolValue(); (bool_ptr && !(*bool_ptr)))
      return CheckState::CERTAIN_FAIL;

    // comparator
    auto comp = [](int i, auto a, auto b)
    {
      switch (i)
      {
        case OP_LESS_EQ:
          return a <= b;
        case OP_GREATER_EQ:
          return a >= b;
        case OP_LESS:
          return a < b;
        case OP_GREATER:
          return a > b;
        case OP_NOT_EQ:
          return a != b;
        case OP_EQ:
          return a == b;
      }
      return false;
    };

    // version comparison
    std::vector<int> app_value_version;

    switch (vs.choice())
    {
      case 0: // Identifier _ Operator _ Version
      {
        // int comparison
        const auto right = std::any_cast<std::vector<int>>(vs[2]);
        if (const auto int_ptr = capability.queryIntValue())
        {
          if (right.size() != 1)
            checkException(vs, "cannot be compared to a version.", capability);

          return comp(op, *int_ptr, right[0]) ? CheckState::CERTAIN_PASS : CheckState::CERTAIN_FAIL;
        }

        const auto string_ptr = capability.queryStringValue();
        if (!string_ptr)
          checkException(vs,
                         "cannot be compared to a " +
                             std::string(right.size() == 1 ? "number" : "version_number") + ".",
                         capability);

        if (!MooseUtils::tokenizeAndConvert(*string_ptr, app_value_version, "."))
          checkException(vs, "cannot be compared to a version.", capability);

        // compare versions
        return comp(op, app_value_version, right) ? CheckState::CERTAIN_PASS
                                                  : CheckState::CERTAIN_FAIL;
      }

      case 1: // Identifier _ Operator _ String
      {
        // here we would check for valid options and throw if not valid
        const auto right = MooseUtils::toLower(std::any_cast<std::string>(vs[2]));
        // the capability value has to be a string
        const auto string_ptr = capability.queryStringValue();
        if (!string_ptr)
          checkException(vs, "cannot be compared to a string.", capability);

        // If this capability has an enumeration, make sure a valid choice is used
        if (!capability.hasEnumeration(right))
          checkException(vs,
                         "'" + right + "' invalid for capability '" + left +
                             "'; valid values: " + capability.enumerationToString());

        // Capability is a version
        if (MooseUtils::tokenizeAndConvert(*string_ptr, app_value_version, "."))
          checkException(vs, "cannot be compared to a string.", capability);

        return comp(op, *string_ptr, right) ? CheckState::CERTAIN_PASS : CheckState::CERTAIN_FAIL;
      }
    }

    checkException(vs, "failed comparison.", capability);
  };

  parser["Bool"] = [this](const SemanticValues & vs)
  {
    // Helper for erroring of a capability doesn't support a boolean
    const auto check_explicit = [&vs](const CapabilityUtils::Capability & capability)
    {
      if (capability.getExplicit())
      {
        std::string message = "capability '" + capability.getName() +
                              "' requires a value and cannot be used in a boolean expression";
        if (capability.hasStringValue() && capability.getEnumeration())
          message += "; valid values: " + capability.enumerationToString();
        checkException(vs, message);
      }
    };

    switch (vs.choice())
    {
      case 0: // Comparison
      case 4: // '(' _ Expression _ ')'
        return std::any_cast<CheckState>(vs[0]);

      case 1: // '!' Bool
        switch (std::any_cast<CheckState>(vs[0]))
        {
          case CheckState::CERTAIN_FAIL:
            return CheckState::CERTAIN_PASS;
          case CheckState::CERTAIN_PASS:
            return CheckState::CERTAIN_FAIL;
          case CheckState::POSSIBLE_FAIL:
            return CheckState::POSSIBLE_PASS;
          case CheckState::POSSIBLE_PASS:
            return CheckState::POSSIBLE_FAIL;
          default:
            return CheckState::UNKNOWN;
        }

      case 2: // '!' Identifier
      {
        if (const auto capability_ptr = query(std::any_cast<std::string>(vs[0])))
        {
          const auto & capability = *capability_ptr;
          if (const auto bool_ptr = capability.queryBoolValue())
            return *bool_ptr ? CheckState::CERTAIN_FAIL : CheckState::CERTAIN_PASS;
          check_explicit(capability);
          return CheckState::CERTAIN_FAIL;
        }
        return CheckState::POSSIBLE_PASS;
      }

      case 3: // Identifier
      {
        if (const auto capability_ptr = query(std::any_cast<std::string>(vs[0])))
        {
          const auto & capability = *capability_ptr;
          if (const auto bool_ptr = capability.queryBoolValue())
            return *bool_ptr ? CheckState::CERTAIN_PASS : CheckState::CERTAIN_FAIL;
          check_explicit(capability);
          return CheckState::CERTAIN_PASS;
        }
        return CheckState::POSSIBLE_FAIL;
      }

      default:
        throw CapabilityException("Unknown choice in Bool non-terminal");
    }
  };

  parser["Expression"] = [](const SemanticValues & vs)
  {
    switch (vs.choice())
    {
      case 0: // Bool _ LogicOperator _ Expression
      {
        const auto left = std::any_cast<CheckState>(vs[0]);
        const auto op = std::any_cast<LogicOperator>(vs[1]);
        const auto right = std::any_cast<CheckState>(vs[2]);

        switch (op)
        {
          case OP_AND:
            for (const auto state : {CheckState::CERTAIN_FAIL,
                                     CheckState::POSSIBLE_FAIL,
                                     CheckState::UNKNOWN,
                                     CheckState::POSSIBLE_PASS,
                                     CheckState::CERTAIN_PASS})
              if (left == state || right == state)
                return state;
            throw CapabilityException("Conjunction failure");

          case OP_OR:
            for (const auto state : {CheckState::CERTAIN_PASS,
                                     CheckState::POSSIBLE_PASS,
                                     CheckState::UNKNOWN,
                                     CheckState::POSSIBLE_FAIL,
                                     CheckState::CERTAIN_FAIL})
              if (left == state || right == state)
                return state;
            throw CapabilityException("Conjunction failure");

          default:
            throw CapabilityException("Unknown logic operator");
        }
      }

      case 1: // Bool
        return std::any_cast<CheckState>(vs[0]);

      default:
        throw CapabilityException("Unknown choice in Expression non-terminal");
    }
  };

  // (4) Parse
  parser.enable_packrat_parsing(); // Enable packrat parsing.

  CheckState state = CheckState::CERTAIN_FAIL;
  if (!parser.parse(requirements, state))
    throw CapabilityException("Unable to parse requested capabilities '", requirements, "'.");

  std::string reason;
  std::string doc;

  return {state, reason, doc};
}
} // namespace CapabilityUtils

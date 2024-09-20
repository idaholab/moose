//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "peglib.h"

#include "CapabilityUtils.h"
#include "MooseUtils.h"
#include "MooseError.h"
#include "Conversion.h"
#include <vector>

namespace CapabilityUtils
{

Result
check(std::string requirements, const Registry & app_capabilities)
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

  parser parser(R"(
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

  mooseAssert(static_cast<bool>(parser) == true, "Capabilities parser build failure.");

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

      default:
        mooseError("Unknown Number match");
    }
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
    mooseError("Unknown logic operator.");
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
    mooseError("Unknown operator '", op, "'.");
  };

  parser["String"] = [](const SemanticValues & vs) { return vs.token_to_string(); };
  parser["Identifier"] = [](const SemanticValues & vs) { return vs.token_to_string(); };

  parser["Comparison"] = [&app_capabilities](const SemanticValues & vs)
  {
    const auto left = std::any_cast<std::string>(vs[0]);
    const auto op = std::any_cast<Operator>(vs[1]);

    // check existence
    const auto it = app_capabilities.find(left);
    if (it == app_capabilities.end())
      // return an unknown if the capability does not exist, this is important as it
      // stays unknown upon negation
      return CheckState::UNKNOWN;

    // capability is registered by the app
    const auto & [app_value, doc] = it->second;

    // explicitly false causes any comparison to fail
    if (std::holds_alternative<bool>(app_value) && std::get<bool>(app_value) == false)
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

    switch (vs.choice())
    {
      case 0: // Identifier _ Operator _ Version
      {
        // int comparison
        const auto right = std::any_cast<std::vector<int>>(vs[2]);
        if (std::holds_alternative<int>(app_value))
        {
          if (right.size() != 1)
            mooseError("Expected an integer value in comparison");

          return comp(op, std::get<int>(app_value), right[0]) ? CheckState::CERTAIN_PASS
                                                              : CheckState::CERTAIN_FAIL;
        }

        // version comparison
        std::vector<int> app_value_version;

        if (!std::holds_alternative<std::string>(app_value))
          mooseError(right.size() == 1
                         ? "Cannot compare capability " + left + " to a number."
                         : "Cannot compare capability " + left + " to a version number.");

        if (!MooseUtils::tokenizeAndConvert(
                std::get<std::string>(app_value), app_value_version, "."))
          mooseError("Expected a version number.");

        // compare versions
        return comp(op, app_value_version, right) ? CheckState::CERTAIN_PASS
                                                  : CheckState::CERTAIN_FAIL;
      }

      case 1: // Identifier _ Operator _ String
      {
        const auto right = std::any_cast<std::string>(vs[2]);
        // the app value has to be a string
        if (!std::holds_alternative<std::string>(app_value))
          mooseError("Unexpected comparison to a string.");

        return comp(op, std::get<std::string>(app_value), MooseUtils::toLower(right))
                   ? CheckState::CERTAIN_PASS
                   : CheckState::CERTAIN_FAIL;
      }
    }

    mooseError("Failed comparison.");
  };

  parser["Bool"] = [&app_capabilities](const SemanticValues & vs)
  {
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
        const auto it = app_capabilities.find(std::any_cast<std::string>(vs[0]));
        if (it != app_capabilities.end())
        {
          const auto app_value = it->second.first;
          if (std::holds_alternative<bool>(app_value) && std::get<bool>(app_value) == false)
            return CheckState::CERTAIN_PASS;
          return CheckState::CERTAIN_FAIL;
        }
        return CheckState::POSSIBLE_PASS;
      }

      case 3: // Identifier
      {
        const auto it = app_capabilities.find(std::any_cast<std::string>(vs[0]));
        if (it != app_capabilities.end())
        {
          const auto app_value = it->second.first;
          if (std::holds_alternative<bool>(app_value) && std::get<bool>(app_value) == false)
            return CheckState::CERTAIN_FAIL;
          return CheckState::CERTAIN_PASS;
        }
        return CheckState::POSSIBLE_FAIL;
      }

      default:
        mooseError("Unknown choice in Bool non-terminal");
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
            mooseError("Conjunction failure");

          case OP_OR:
            for (const auto state : {CheckState::CERTAIN_PASS,
                                     CheckState::POSSIBLE_PASS,
                                     CheckState::UNKNOWN,
                                     CheckState::POSSIBLE_FAIL,
                                     CheckState::CERTAIN_FAIL})
              if (left == state || right == state)
                return state;
            mooseError("Conjunction failure");

          default:
            mooseError("Unknown logic operator");
        }
      }

      case 1: // Bool
        return std::any_cast<CheckState>(vs[0]);

      default:
        mooseError("Unknown choice in Expression non-terminal");
    }
  };

  // (4) Parse
  parser.enable_packrat_parsing(); // Enable packrat parsing.

  CheckState state = CheckState::CERTAIN_FAIL;
  if (!parser.parse(requirements, state))
    mooseError("Unable to parse requirements '", requirements, "'.");

  std::string reason;
  std::string doc;

  return {state, reason, doc};
}
} // namespace CapabilityUtils

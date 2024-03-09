//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CapabilityUtils.h"
#include "MooseUtilsStandalone.h"
#include <vector>

namespace CapabilityUtils
{

Result
check(const std::string & requirements, const Registry & capabilities)
{
  // results from each condition
  std::vector<Result> results;

  // break up list into individual capability checks
  std::vector<std::string> conditions;
  MooseUtils::tokenize<std::string>(MooseUtils::toLower(requirements), conditions, 1, " ");

  // operators (order matters to avoid matching < in 'a<=b')
  static const std::vector<std::string> ops = {"<=", ">=", "<", ">", "!=", "==", "="};
  // comparator
  auto comp = [](std::size_t i, auto a, auto b)
  {
    switch (i)
    {
      case 0:
        return a <= b;
      case 1:
        return a >= b;
      case 2:
        return a < b;
      case 3:
        return a > b;
      case 4:
        return a != b;
      case 5:
      case 6:
        return a == b;
    }
    return false;
  };

  for (const auto & condition : conditions)
  {
    // is the current condition negated?
    bool negate = (condition[0] == '!');

    // check if an operator is used
    std::size_t op = 0;
    std::size_t pos = std::string::npos;
    for (; op < ops.size(); ++op)
      if (pos = condition.find(ops[op]); pos != std::string::npos)
        break;
    std::string capability, test_value;

    if (op == ops.size())
    {
      // no operator
      capability = condition.substr(negate);
      test_value = "";
    }
    else
    {
      capability = condition.substr(negate, pos - negate);
      test_value = condition.substr(pos + ops[op].size());
      if (test_value.empty())
        throw std::invalid_argument(condition + " is missing a right hand side");
    }

    // simple existence non-false check (boolean)
    const auto it = capabilities.find(capability);
    if (it == capabilities.end())
    {
      // capability is not registered at all
      results.emplace_back(negate ? POSSIBLE_PASS : POSSIBLE_FAIL,
                           capability + " not supported.",
                           "This input likely needs to be run with a different MOOSE "
                           "application, or requires dynamically linking another module or "
                           "application. Please also make sure your applications are up to date. ");
      continue;
    }

    // capability is registered by the app
    const auto & [app_value, doc] = it->second;

    // if no operator is found we can check for existence where it shouldn't exist
    if (op == ops.size())
    {
      if (std::holds_alternative<bool>(app_value))
      {
        results.emplace_back(std::get<bool>(app_value) != negate ? CERTAIN_PASS : CERTAIN_FAIL,
                             capability + (negate ? " supported" : " not supported"),
                             doc);
        continue;
      }

      results.emplace_back(negate ? CERTAIN_FAIL : CERTAIN_PASS,
                           capability + (negate ? " supported" : " not supported"),
                           doc);
      continue;
    }

    if (std::holds_alternative<bool>(app_value) && std::get<bool>(app_value) == negate)
    {
      results.emplace_back(
          CERTAIN_FAIL, capability + (negate ? " supported" : " not supported"), doc);
      continue;
    }

    // if there is no operator we're done here
    if (op == ops.size())
      continue;

    // int comparison
    if (std::holds_alternative<int>(app_value) &&
        comp(op, std::get<int>(app_value), std::stoi(test_value)) == negate)
    {
      results.emplace_back(CERTAIN_FAIL, condition + " not fulfilled", doc);
    }

    // string comparison
    if (std::holds_alternative<std::string>(app_value))
    {
      // check for version number
      std::vector<int> version1, version2;
      if (MooseUtils::tokenizeAndConvert(std::get<std::string>(app_value), version1, ".") &&
          MooseUtils::tokenizeAndConvert(test_value, version2, "."))
      {
        if (comp(op, version1, version2) == negate)
        {
          results.emplace_back(CERTAIN_FAIL, condition + " version not matched", doc);
          continue;
        }
      }
      else
      {
        if (comp(op, std::get<std::string>(app_value), test_value) == negate)
        {
          results.emplace_back(CERTAIN_FAIL, condition + " not fulfilled", doc);
          continue;
        }
      }
    }
  }

  // reduce result
  CheckState state = CERTAIN_PASS;
  std::string reason;
  std::string doc;
  for (const auto & [item_state, item_reason, item_doc] : results)
  {
    state = std::min(state, item_state);
    if (item_state <= POSSIBLE_FAIL)
    {
      reason += (doc.empty() ? "" : ", ") + item_reason;
      doc += (doc.empty() ? "" : " ") + item_doc;
    }
  }

  return {state, reason, doc};
}

} // namespace CapabilityUtils

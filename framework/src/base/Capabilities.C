//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Capabilities.h"
#include "MooseUtils.h"

#include "nlohmann/json.h"

#include <vector>

namespace Moose
{

Capabilities &
Capabilities::instance()
{
  // We need a naked new here (_not_ a smart pointer or object instance) due to what seems like a
  // bug in clang's static object destruction when using dynamic library loading.
  static Capabilities * instance = nullptr;
  if (!instance)
    instance = new Capabilities;
  return *instance;
}

void
Capabilities::add(const std::string & capability, CapabilityType value, const std::string & doc)
{
  instance().addInternal(capability, value, doc);
}

void
Capabilities::add(const std::string & capability, const char * value, const char * doc)
{
  instance().addInternal(capability, std::string(value), std::string(doc));
}

std::string
Capabilities::dump()
{
  return instance().dumpInternal();
}

std::pair<bool, std::string>
Capabilities::check(const std::string & requested_capabilities)
{
  return instance().checkInternal(requested_capabilities);
}

void
Capabilities::addInternal(const std::string & raw_capability,
                          CapabilityType value,
                          const std::string & doc)
{
  const auto capability = MooseUtils::toLower(raw_capability);
  if (std::holds_alternative<std::string>(value))
    value = MooseUtils::toLower(std::get<std::string>(value));

  auto it_pair = _capability_registry.lower_bound(capability);
  if (it_pair == _capability_registry.end() || it_pair->first != capability ||
      it_pair->second.first == value)
    it_pair = _capability_registry.emplace_hint(it_pair, capability, std::make_pair(value, doc));
  else
    mooseWarning("Capability '", capability, "' was already registered with a different value.");
}

std::string
Capabilities::dumpInternal() const
{
  nlohmann::json root;
  for (const auto & [capability, value_doc] : _capability_registry)
  {
    const auto & value = value_doc.first;
    if (std::holds_alternative<bool>(value))
      root[capability] = std::get<bool>(value);
    else if (std::holds_alternative<int>(value))
      root[capability] = std::get<int>(value);
    else if (std::holds_alternative<std::string>(value))
      root[capability] = std::get<std::string>(value);
    else
      mooseError("Unknown type in capabilities registry");
  }
  return root.dump(2);
}

std::pair<bool, std::string>
Capabilities::checkInternal(const std::string & requested_capabilities) const
{
  // break up list into individual capability checks
  std::vector<std::string> conditions;
  MooseUtils::tokenize<std::string>(
      MooseUtils::toLower(requested_capabilities), conditions, 1, " ");

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
    }

    // simple existence non-false check (boolean)
    const auto it = _capability_registry.find(capability);
    if (it == _capability_registry.end())
      // capability is not registered at all
      return {negate,
              capability +
                  " not supported | This input is likely needs to be run with a different MOOSE "
                  "application, or requires dynamically linking another module or "
                  "application. Please also make sure your applications are up to date."};

    // capability is registered by the app
    const auto & [app_value, doc] = it->second;

    // if no operator is found we can check for existence where it shouldn't exist
    if (op == ops.size())
    {
      if (std::holds_alternative<bool>(app_value))
        return {std::get<bool>(app_value) != negate,
                capability + (negate ? " supported" : " not supported") + " | " + doc};

      return {!negate, capability + (negate ? " supported" : " not supported") + " | " + doc};
    }

    if (std::holds_alternative<bool>(app_value) && std::get<bool>(app_value) == negate)
      return {false, capability + (negate ? " supported" : " not supported") + " | " + doc};

    // if there is no operator we're done here
    if (op == ops.size())
      continue;

    // int comparison
    if (std::holds_alternative<int>(app_value) &&
        comp(op, std::get<int>(app_value), std::stoi(test_value)) == negate)
      return {false, condition + " not fulfilled" + " | " + doc};

    // string comparison
    if (std::holds_alternative<std::string>(app_value))
    {
      // error
      if (test_value.empty())
        return {false, condition + " broken"};

      // check for version number
      std::vector<int> version1, version2;
      if (MooseUtils::tokenizeAndConvert(std::get<std::string>(app_value), version1, ".") &&
          MooseUtils::tokenizeAndConvert(test_value, version2, "."))
      {
        if (comp(op, version1, version2) == negate)
          return {false, condition + " version not matched | " + doc};
      }
      else
      {
        if (comp(op, std::get<std::string>(app_value), test_value) == negate)
          return {false, condition + " not fulfilled | " + doc};
      }
    }
  }

  return {true, ""};
}

} // namespace Moose

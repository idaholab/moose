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
#include "Conversion.h"

#include "nlohmann/json.h"

#include <vector>

namespace Moose
{

Capabilities &
Capabilities::getCapabilityRegistry()
{
  // We need a naked new here (_not_ a smart pointer or object instance) due to what seems like a
  // bug in clang's static object destruction when using dynamic library loading.
  static Capabilities * capability_registry = nullptr;
  if (!capability_registry)
    capability_registry = new Capabilities();
  return *capability_registry;
}

void
Capabilities::add(const std::string & raw_capability,
                  CapabilityUtils::Type value,
                  const std::string & doc)
{
  const auto capability = MooseUtils::toLower(raw_capability);
  if (std::holds_alternative<std::string>(value))
    value = MooseUtils::toLower(std::get<std::string>(value));

  auto it_pair = _capability_registry.lower_bound(capability);
  if (it_pair == _capability_registry.end() || it_pair->first != capability ||
      it_pair->second.first == value)
    _capability_registry.emplace_hint(it_pair, capability, std::make_pair(value, doc));
  else
    mooseError("Capability '",
               capability,
               "' was already registered with a different value. ('",
               Moose::stringify(it_pair->second.first),
               "' instead of '",
               Moose::stringify(value),
               "')");
}

void
Capabilities::add(const std::string & capability, const char * value, const char * doc)
{
  add(capability, std::string(value), std::string(doc));
}

std::string
Capabilities::dump() const
{
  nlohmann::json root;
  for (const auto & [capability, value_doc] : _capability_registry)
  {
    const auto & value = value_doc.first;
    const auto & doc = value_doc.second;
    if (std::holds_alternative<bool>(value))
      root[capability] = {std::get<bool>(value), doc};
    else if (std::holds_alternative<int>(value))
      root[capability] = {std::get<int>(value), doc};
    else if (std::holds_alternative<std::string>(value))
      root[capability] = {std::get<std::string>(value), doc};
    else
      mooseError("Unknown type in capabilities registry");
  }
  return root.dump(2);
}

CapabilityUtils::Result
Capabilities::check(const std::string & requested_capabilities) const
{
  const auto result = CapabilityUtils::check(requested_capabilities, _capability_registry);
  if (std::get<0>(result) == CapabilityUtils::PARSE_FAIL)
    mooseError("Unable to parse requested capabilities '", requested_capabilities, "'.");
  return result;
}

} // namespace Moose

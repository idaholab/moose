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
Capabilities::add(const std::string & capability, CapabilityType value)
{
  instance().addInternal(capability, value);
}

std::string
Capabilities::dump()
{
  return instance().dumpInternal();
}

void
Capabilities::addInternal(const std::string & raw_capability, CapabilityType value)
{
  const auto capability = MooseUtils::toLower(raw_capability);
  if (std::holds_alternative<std::string>(value))
    value = MooseUtils::toLower(std::get<std::string>(value));

  auto it_pair = _capability_registry.lower_bound(capability);
  if (it_pair == _capability_registry.end() || it_pair->first != capability)
    it_pair = _capability_registry.emplace_hint(it_pair, capability, value);
  else
    mooseWarning("Capability '", capability, "' was already registered.");
}

std::string
Capabilities::dumpInternal() const
{
  nlohmann::json root;
  for (const auto & [capability, value] : _capability_registry)
    if (std::holds_alternative<bool>(value))
      root[capability] = std::get<bool>(value);
    else if (std::holds_alternative<int>(value))
      root[capability] = std::get<int>(value);
    else if (std::holds_alternative<std::string>(value))
      root[capability] = std::get<std::string>(value);
    else
      mooseError("Unknown type in capabilities registry");
  return root.dump(2);
}

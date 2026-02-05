//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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

const std::set<std::string> Capabilities::reserved_augmented_capabilities{
    // TestHarness.getCapabilities()
    "hpc",
    "machine",
    "library_mode",
    // RunApp.getAugmentedCapabilities()
    "mpi_procs",
    "num_threads"};

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

CapabilityUtils::Capability &
Capabilities::add(const std::string_view capability,
                  const CapabilityUtils::CapabilityValue & value,
                  const std::string_view doc)
{
  auto & cap = _capability_registry.add(capability, value, doc);

  if (reserved_augmented_capabilities.count(cap.getName()))
    throw CapabilityUtils::CapabilityException(
        "The capability \"" + cap.getName() +
        "\" is reserved and may not be registered by an application.");

  return cap;
}

std::string
Capabilities::dump() const
{
  nlohmann::json root;
  for (const auto & [name, capability] : _capability_registry.getRegistry())
  {
    auto & entry = root[name];
    std::visit([&entry](const auto & v) { entry["value"] = v; }, capability.getValue());
    entry["doc"] = capability.getDoc();
    if (capability.hasStringValue())
      if (const auto & enumeration_ptr = capability.getEnumeration())
        entry["enumeration"] = *enumeration_ptr;
    if (!capability.hasBoolValue())
      entry["explicit"] = capability.getExplicit();
  }
  return root.dump(2);
}

CapabilityUtils::CapabilityRegistry::Result
Capabilities::check(const std::string & requested_capabilities) const
{
  return _capability_registry.check(requested_capabilities);
}

void
Capabilities::augment(const nlohmann::json & input, const Capabilities::AugmentPassKey)
{
  for (const auto & [name_json, entry] : input.items())
  {
    const std::string name = name_json;

    const std::string doc = entry["doc"];

    CapabilityUtils::CapabilityValue value;
    const auto & value_json = entry["value"];
    if (value_json.is_boolean())
      value = value_json.get<bool>();
    else if (value_json.is_number_integer())
      value = value_json.get<int>();
    else
      value = value_json.get<std::string>();

    auto & capability = _capability_registry.add(name, value, doc);

    if (entry.contains("explicit") && entry["explicit"].get<bool>())
      capability.setExplicit();

    if (entry.contains("enumeration"))
      capability.setEnumeration(entry["enumeration"].get<std::vector<std::string>>());
  }
}

} // namespace Moose

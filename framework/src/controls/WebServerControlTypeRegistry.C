//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WebServerControlTypeRegistry.h"

namespace Moose
{
WebServerControlTypeRegistry &
WebServerControlTypeRegistry::getRegistry()
{
  static WebServerControlTypeRegistry * registry_singleton = nullptr;
  if (!registry_singleton)
    registry_singleton = new WebServerControlTypeRegistry;
  return *registry_singleton;
}

const WebServerControlTypeRegistry::RegisteredTypeBase &
WebServerControlTypeRegistry::get(const std::string & type)
{
  auto & registry = getRegistry();
  const auto it = registry._name_map.find(type);
  if (it == registry._name_map.end())
    mooseError("WebServerControlTypeRegistry: The type '", type, "' is not registered");
  return *it->second;
}
}

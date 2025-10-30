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

const WebServerControlTypeRegistry::RegisteredTypeBase *
WebServerControlTypeRegistry::query(const std::string & type)
{
  const auto & name_map = getRegistry()._name_map;
  if (const auto it = name_map.find(type); it != name_map.end())
  {
    mooseAssert(it->second, "Item is nullptr");
    return it->second.get();
  }
  return nullptr;
}

const WebServerControlTypeRegistry::RegisteredTypeBase &
WebServerControlTypeRegistry::get(const std::string & type)
{
  if (const auto obj = query(type))
    return *obj;
  mooseError("WebServerControlTypeRegistry: The type '", type, "' is not registered");
}

WebServerControlTypeRegistry::RegisteredTypeBase::RegisteredTypeBase(const std::string & type)
  : _type(type)
{
}

WebServerControlTypeRegistry::ControlledValueBase::ControlledValueBase(const std::string & name,
                                                                       const std::string & type)
  : _name(name), _type(type)
{
}
}

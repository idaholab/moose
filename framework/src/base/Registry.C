//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InputParameters.h"
#include "Registry.h"
#include "Factory.h"
#include "ActionFactory.h"

#include "libmesh/libmesh_common.h"

#include <memory>

Registry &
Registry::getRegistry()
{
  static Registry registry_singleton;
  return registry_singleton;
}

void
Registry::addInner(const RegistryEntry & info)
{
  getRegistry()._per_label_objects[info._label].push_back(info);
}

void
Registry::addActionInner(const RegistryEntry & info)
{
  getRegistry()._per_label_actions[info._label].push_back(info);
}

void
Registry::registerObjectsTo(Factory & f, const std::set<std::string> & labels)
{
  auto & r = getRegistry();

  for (const auto & label : labels)
  {
    r._known_labels.insert(label);
    if (r._per_label_objects.count(label) == 0)
      continue;

    for (const auto & obj : r._per_label_objects[label])
    {
      std::string name = obj._name;
      if (name.empty())
        name = obj._alias;
      if (name.empty())
        name = obj._classname;

      r._name_to_entry[name] = obj;

      f.reg(obj._label,
            name,
            obj._build_ptr,
            obj._params_ptr,
            obj._deprecated_time,
            obj._replaced_by,
            obj._file,
            obj._line);

      if (!obj._alias.empty())
        f.associateNameToClass(name, obj._classname);
    }
  }
}

RegistryEntry &
Registry::objData(const std::string & name)
{
  auto & r = getRegistry();

  auto it = r._name_to_entry.find(name);

  if (it != r._name_to_entry.end())
    return it->second;
  else
    mooseError("Object ", name, " is not registered yet");
}

void
Registry::registerActionsTo(ActionFactory & f, const std::set<std::string> & labels)
{
  auto & r = getRegistry();

  for (const auto & label : labels)
  {
    r._known_labels.insert(label);
    if (r._per_label_actions.count(label) == 0)
      continue;

    for (const auto & obj : r._per_label_actions[label])
      f.reg(
          obj._classname, obj._name, obj._build_action_ptr, obj._params_ptr, obj._file, obj._line);
  }
}

char
Registry::addKnownLabel(const std::string & label)
{
  getRegistry()._known_labels.insert(label);
  return 0;
}

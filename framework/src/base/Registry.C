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
#include "MooseUtils.h"

#include "libmesh/libmesh_common.h"

#include <memory>

Registry &
Registry::getRegistry()
{
  // We need a naked new here (_not_ a smart pointer or object instance) due to what seems like a
  // bug in clang's static object destruction when using dynamic library loading.
  static Registry * registry_singleton = nullptr;
  if (!registry_singleton)
    registry_singleton = new Registry();
  return *registry_singleton;
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
      const auto name = obj->name();
      r._name_to_entry[name] = obj;

      f.reg(obj);
      if (!obj->_alias.empty())
        f.associateNameToClass(name, obj->_classname);
    }
  }
}

const RegistryEntryBase &
Registry::objData(const std::string & name)
{
  auto & r = getRegistry();

  if (const auto it = r._name_to_entry.find(name); it != r._name_to_entry.end())
    return *it->second;
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
      f.reg(obj);
  }
}

char
Registry::addKnownLabel(const std::string & label)
{
  getRegistry()._known_labels.insert(label);
  return 0;
}

void
Registry::addDataFilePath(const std::string & fullpath)
{
  auto & dfp = getRegistry()._data_file_paths;

  // split the *App.C filename from its containing directory
  const auto path = MooseUtils::splitFileName(fullpath).first;

  // This works for both build/unity_src/ and src/base/ as the *App.C file location,
  // in case __FILE__ doesn't get overriden in unity build
  const auto data_dir = MooseUtils::pathjoin(path, "../../data");

  // if the data directory exists and hasn't been added before, add it
  if (MooseUtils::pathIsDirectory(data_dir) &&
      std::find(dfp.begin(), dfp.end(), data_dir) == dfp.end())
    dfp.push_back(data_dir);
}

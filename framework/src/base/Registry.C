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

static Registry &
getRegistry()
{
  static std::unique_ptr<Registry> _singleton;
  if (!_singleton)
    _singleton = libmesh_make_unique<Registry>();
  return *_singleton;
}

const std::map<std::string, std::vector<RegistryEntry>> &
Registry::allObjects()
{
  return getRegistry()._per_label_objects;
}
const std::map<std::string, std::vector<RegistryEntry>> &
Registry::allActions()
{
  return getRegistry()._per_label_actions;
}

void
Registry::addInner(const RegistryEntry & info)
{
  auto & r = getRegistry();
  r._per_label_objects[info._label].push_back(info);
}

void
Registry::addActionInner(const RegistryEntry & info)
{
  auto & r = getRegistry();
  r._per_label_actions[info._label].push_back(info);
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

void
Registry::checkLabels(const std::set<std::string> & known_labels)
{
  auto & r = getRegistry();
  std::vector<RegistryEntry> orphs;

  for (auto & entry : r._per_label_objects)
    if (known_labels.count(entry.first) == 0 && r._known_labels.count(entry.first) == 0)
      orphs.insert(orphs.end(), entry.second.begin(), entry.second.end());
  for (auto & entry : r._per_label_actions)
    if (known_labels.count(entry.first) == 0 && r._known_labels.count(entry.first) == 0)
      orphs.insert(orphs.end(), entry.second.begin(), entry.second.end());

  if (orphs.size() > 0)
  {
    std::stringstream lst;
    for (auto & orph : orphs)
      lst << "\n\t" << orph._classname << " (app='" << orph._label << "')";
    mooseError("The following objects/actions have been registered to unknown applications/labels:",
               lst.str());
  }
}

char
Registry::addKnownLabel(const std::string & label)
{
  auto & r = getRegistry();
  r._known_labels.insert(label);
  return 0;
}

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
#include <filesystem>
#include <regex>

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
Registry::addDataFilePath(const std::string & name, const std::string & in_tree_path)
{
  if (!std::regex_search(name, std::regex("\\w+")))
    mooseError("Unallowed characters in '", name, "'");

  // Enforce that the folder is called "data", because we rely on the installed path
  // to be within PREFIX/share/<name>/data (see determineDataFilePath())
  const std::string folder = std::filesystem::path(in_tree_path).filename().c_str();
  if (folder != "data")
    mooseError("While registering data file path '",
               in_tree_path,
               "' for '",
               name,
               "': The folder must be named 'data' and it is named '",
               folder,
               "'");

  // Find either the installed or in-tree path
  const auto path = determineDataFilePath(name, in_tree_path);

  auto & dfp = getRegistry()._data_file_paths;
  const auto it = dfp.find(name);
  // Not registered yet
  if (it == dfp.end())
    dfp.emplace(name, path);
  // Registered, but with a different value
  else if (it->second != path)
    mooseError("While registering data file path '",
               path,
               "' for '",
               name,
               "': the path '",
               it->second,
               "' is already registered");
}

void
Registry::addAppDataFilePath(const std::string & app_name, const std::string & app_path)
{
  // split the *App.C filename from its containing directory
  const auto dir = MooseUtils::splitFileName(app_path).first;
  // This works for both build/unity_src/ and src/base/ as the *App.C file location,
  // in case __FILE__ doesn't get overriden in unity build
  addDataFilePath(app_name, MooseUtils::pathjoin(dir, "../../data"));
}

void
Registry::addDeprecatedAppDataFilePath(const std::string & app_path)
{
  const auto app_name = appNameFromAppPath(app_path);
  mooseDeprecated("In ",
                  app_path,
                  ":\nregisterDataFilePath() is deprecated. Use registerAppDataFilePath(\"",
                  app_name,
                  "\") instead.");
  addAppDataFilePath(app_name, app_path);
}

std::string
Registry::getDataFilePath(const std::string & name)
{
  const auto & dfps = getRegistry()._data_file_paths;
  const auto it = dfps.find(name);
  if (it == dfps.end())
    mooseError("Registry::getDataFilePath(): A data file path for '", name, "' is not registered");
  return it->second;
}

void
Registry::addRepository(const std::string & repo_name, const std::string & repo_url)
{
  auto & repos = getRegistry()._repos;
  const auto [it, inserted] = repos.emplace(repo_name, repo_url);
  if (!inserted && it->second != repo_url)
    mooseError("Registry::registerRepository(): The repository '",
               repo_name,
               "' is already registered with a different URL '",
               it->second,
               "'.");
}

const std::string &
Registry::getRepositoryURL(const std::string & repo_name)
{
  const auto & repos = getRegistry()._repos;
  if (const auto it = repos.find(repo_name); it != repos.end())
    return it->second;
  mooseError("Registry::getRepositoryURL(): The repository '", repo_name, "' is not registered.");
}

std::string
Registry::determineDataFilePath(const std::string & name, const std::string & in_tree_path)
{
  // TODO: Track whether or not the application is installed in a better way
  // than this, which will enable us to pick one or the other based on
  // the install state. This probably also won't work with dynamic loading, where
  // we can't necessarily get this information from the binary (as there could be
  // multiple binary paths)

  // Installed data
  const auto installed_path =
      MooseUtils::pathjoin(Moose::getExecutablePath(), "..", "share", name, "data");
  if (MooseUtils::checkFileReadable(installed_path, false, false, false))
    return MooseUtils::canonicalPath(installed_path);

  // In tree data
  if (MooseUtils::checkFileReadable(in_tree_path, false, false, false))
    return MooseUtils::canonicalPath(in_tree_path);

  mooseError("Failed to determine data file path for '",
             name,
             "'. Paths searched:\n\n  installed: ",
             installed_path,
             "\n  in-tree: ",
             in_tree_path);
}

std::string
Registry::appNameFromAppPath(const std::string & app_path)
{
  // This is for deprecated use only. It assumes that the application name
  // (binary name) in the build follows our normal naming of FooBarApp -> foo_bar.
  // We need to convert the application source file to the above, for example:
  //   /path/to/FooBarBazApp.C -> foo_bar_baz
  // Ideally, we would instead have the user specify this manually so that
  // there is no ambiguity.
  std::smatch match;
  if (std::regex_search(app_path, match, std::regex("\\/([a-zA-Z0-9_]+)App\\.C$")))
  {
    std::string name = match[1];                                        // FooBarBaz
    name = std::regex_replace(name, std::regex("(?!^)([A-Z])"), "_$1"); // Foo_Bar_Baz
    name = MooseUtils::toLower(name);                                   // foo_bar_baz
    return name;
  }

  mooseError(
      "Registry::appNameFromAppPath(): Failed to parse application name from '", app_path, "'");
}

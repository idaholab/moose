//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InputParameters.h"

#include <string>
#include <vector>
#include <set>
#include <map>
#include <memory>

#include "libmesh/utility.h"

#include <gtest/gtest.h>

#define combineNames1(X, Y) X##Y
#define combineNames(X, Y) combineNames1(X, Y)

/// This is provided as a convenience to globally set certain app names or labels used for
/// objects/actions as allowable.  While usually not needed, this macro can be useful for cases
/// when your app/module code may be compiled with other apps without your objects being
/// registered.  Calling this multiple times with the same argument is safe.
#define registerKnownLabel(X)                                                                      \
  static char combineNames(dummy_var_for_known_label, __COUNTER__) = Registry::addKnownLabel(X)

/// add an Action to the registry with the given app name/label as being associated with the given
/// task (quoted string).  classname is the (unquoted) c++ class.
#define registerMooseAction(app, classname, task)                                                  \
  static char combineNames(dummyvar_for_registering_action_##classname, __COUNTER__) =             \
      Registry::addAction<classname>({app, #classname, "", task, __FILE__, __LINE__, "", ""})

/// Add a MooseObject to the registry with the given app name/label.  classname is the (unquoted)
/// c++ class.  Each object/class should only be registered once.
#define registerMooseObject(app, classname)                                                        \
  static char combineNames(dummyvar_for_registering_obj_##classname, __COUNTER__) =                \
      Registry::add<classname>({app, #classname, "", "", __FILE__, __LINE__, "", ""})

#define registerADMooseObject(app, classname) registerMooseObject(app, classname)

/// Add a MooseObject to the registry with the given app name/label under an alternate alias/name
/// (quoted string) instead of the classname.
#define registerMooseObjectAliased(app, classname, alias)                                          \
  static char combineNames(dummyvar_for_registering_obj_##classname, __COUNTER__) =                \
      Registry::add<classname>({app, #classname, alias, "", __FILE__, __LINE__, "", ""})

/// Add a deprecated MooseObject to the registry with the given app name/label. time is the time
/// the object became/becomes deprecated in "mm/dd/yyyy HH:MM" format.
#define registerMooseObjectDeprecated(app, classname, time)                                        \
  static char combineNames(dummyvar_for_registering_obj_##classname, __COUNTER__) =                \
      Registry::add<classname>({app, #classname, "", "", __FILE__, __LINE__, time, ""})

#define registerADMooseObjectDeprecated(app, classname, time)                                      \
  registerMooseObjectDeprecated(app, classname, time)

/// add a deprecated MooseObject to the registry that has been replaced by another
/// object. time is the time the object became/becomes deprecated in "mm/dd/yyyy hh:mm" format.
#define registerMooseObjectReplaced(app, classname, time, replacement)                             \
  static char combineNames(dummyvar_for_registering_obj_##classname, __COUNTER__) =                \
      Registry::add<classname>({app, #classname, "", "", __FILE__, __LINE__, time, #replacement})

/// add a deprecated MooseObject orig_class to the registry that has been replaced by another
/// object new_class with the same API. time is the time the object became/becomes deprecated in
/// "mm/dd/yyyy hh:mm" format.
/// A call to registerMooseObject is still required for the new class
#define registerMooseObjectRenamed(app, orig_class, time, new_class)                               \
  static char combineNames(dummyvar_for_registering_obj_##orig_class, __COUNTER__) =               \
      Registry::add<new_class>(                                                                    \
          {app, #new_class, #orig_class, #orig_class, __FILE__, __LINE__, time, #new_class})

#define registerADMooseObjectRenamed(app, orig_class, time, new_class)                             \
  registerMooseObjectRenamed(app, orig_class, time, new_class)

/// Register a non-MooseApp data file path (folder name must be data)
#define registerNonAppDataFilePath(name, path) Registry::addDataFilePath(name, path)
/// Register a data file path for an application. Uses the current file to register
/// ../../data as a path. The app name must be the APPLICATION_NAME used to build
/// the app (solid_mechanics instead of SolidMechanicsApp, for example)
#define registerAppDataFilePath(app) Registry::addAppDataFilePath(app, __FILE__)
/// Deprecated method; use registerAppDataFilePath instead
#define registerDataFilePath() Registry::addDeprecatedAppDataFilePath(__FILE__)

#define registerRepository(repo_name, repo_url) Registry::addRepository(repo_name, repo_url);

class Factory;
class ActionFactory;
class MooseObject;
class Action;
struct RegistryEntryBase;

/**
 * Holds details and meta-data info for a particular MooseObject or Action for use in the
 * use in the registry.
 */
struct RegistryEntryData
{
  /// label (usually app name - e.g. "YourAnimalApp") that the object or action is associated with.
  std::string _label;
  /// name of the c++ class for the object.
  std::string _classname;
  /// an alternate name to register the object to factories under.
  /// If unspecified, _classname is used.
  std::string _alias;
  /// name that the object will be registered to factories under.  If unspecified, _alias is used.
  std::string _name;
  /// file path for the c++ file the object or action was added to the registry in.
  std::string _file;
  /// line number in the c++ file the object or action was added to the registry on.
  int _line;
  /// time in "mm/dd/yyyy HH:MM" format that the object is/becomes deprecated, blank otherwise.
  std::string _deprecated_time;
  /// class name for an object that replaces this object if deprecated, blank otherwise.
  std::string _replaced_by;
};

struct RegistryEntryBase : public RegistryEntryData
{
  RegistryEntryBase(const RegistryEntryData & data) : RegistryEntryData(data) {}
  virtual ~RegistryEntryBase() {}
  /// proxy functions
  virtual std::unique_ptr<MooseObject> build(const InputParameters & parameters) = 0;
  virtual std::shared_ptr<Action> buildAction(const InputParameters & parameters) = 0;
  virtual InputParameters buildParameters() = 0;
  /// resolve the name from _classname, _alias, and _name
  std::string name() const
  {
    std::string name = _name;
    if (name.empty())
      name = _alias;
    if (name.empty())
      name = _classname;
    return name;
  }
};

template <typename T>
struct RegistryEntry : public RegistryEntryBase
{
  RegistryEntry(const RegistryEntryData & data) : RegistryEntryBase(data) {}
  virtual std::unique_ptr<MooseObject> build(const InputParameters & parameters) override;
  virtual std::shared_ptr<Action> buildAction(const InputParameters & parameters) override;
  virtual InputParameters buildParameters() override;
};

/// The registry is used as a global singleton to collect information on all available MooseObject
/// and Action classes for use in a moose app/simulation.  It must be global because we want+need
/// to be able to register objects in global scope during static initialization time before other
/// parts of the moose app execution have started running.  This allows us to distribute
/// registration across all the files that define the actual classes being registered so we don't
/// have to have any central location with a bajillion includes that makes (especially incremental)
/// compiles slow. The registry collects the app, name, and other information for each objects and
/// makes it available to the moose object and action factories and others for general use.  All
/// public functions in this class modify and return data from the global singleton.
class Registry
{
public:
  /**
   * Get the global Registry singleton.
   */
  static Registry & getRegistry();

  /// Adds information on a MooseObject to the registry.  The _build_ptr, _build_action_ptr, and
  /// _params_ptr objects of the info object should all be nullptr - these are set automatically by
  /// the add function itself using the templated type T.
  template <typename T>
  static char add(const RegistryEntryData & base_info)
  {
    const auto info = std::make_shared<RegistryEntry<T>>(base_info);
    getRegistry()._per_label_objects[info->_label].push_back(info);
    getRegistry()._type_to_classname[typeid(T).name()] = info->name();
    return 0;
  }

  /// Adds information on an Action object to the registry.  The _build_ptr, _build_action_ptr, and
  /// _params_ptr objects of the info object should all be nullptr - these are set automatically by
  /// the addAction function itself using the templated type T.
  template <typename T>
  static char addAction(const RegistryEntryData & base_info)
  {
    const auto info = std::make_shared<RegistryEntry<T>>(base_info);
    getRegistry()._per_label_actions[info->_label].push_back(info);
    getRegistry()._type_to_classname[typeid(T).name()] = info->_classname;
    return 0;
  }

  template <typename T>
  static std::string getClassName()
  {
    return libmesh_map_find(getRegistry()._type_to_classname, typeid(T).name());
  }

  /// This registers all MooseObjects known to the registry that have the given label(s) with the
  /// factory f.
  static void registerObjectsTo(Factory & f, const std::set<std::string> & labels);

  /// This registers all Actions known to the registry that have the given label(s) with the
  /// factory f.
  static void registerActionsTo(ActionFactory & f, const std::set<std::string> & labels);

  /// addKnownLabel whitelists a label as valid for purposes of the checkLabels function.
  static char addKnownLabel(const std::string & label);

  /// register general search paths (folder name must be data)
  static void addDataFilePath(const std::string & name, const std::string & in_tree_path);
  /// register search paths for an application (path determined relative to app_path);
  /// app_path should be passed as __FILE__ from the application source file
  static void addAppDataFilePath(const std::string & app_name, const std::string & app_path);
  /// deprecated method; use addAppDataFilePath instead
  static void addDeprecatedAppDataFilePath(const std::string & app_path);

  /// register a repository
  static void addRepository(const std::string & repo_name, const std::string & repo_url);

  /// Returns a per-label keyed map of all MooseObjects in the registry.
  static const std::map<std::string, std::vector<std::shared_ptr<RegistryEntryBase>>> & allObjects()
  {
    return getRegistry()._per_label_objects;
  }
  /// Returns a per-label keyed map of all Actions in the registry.
  static const std::map<std::string, std::vector<std::shared_ptr<RegistryEntryBase>>> & allActions()
  {
    return getRegistry()._per_label_actions;
  }

  static const RegistryEntryBase & objData(const std::string & name);

  /**
   * \returns true if an object with the given name is registered
   */
  static bool isRegisteredObj(const std::string & name)
  {
    return getRegistry()._name_to_entry.count(name);
  }

  /// Returns a map of all registered data file paths (name -> path)
  static const std::map<std::string, std::string> & getDataFilePaths()
  {
    return getRegistry()._data_file_paths;
  }
  /**
   * Gets a data path for the registered name.
   *
   * Finds either the installed path or the in-tree path.
   */
  static std::string getDataFilePath(const std::string & name);

  /// Returns the repository URL associated with \p repo_name
  static const std::string & getRepositoryURL(const std::string & repo_name);
  /**
   * Returns a map of all registered repositories
   */
  static const std::map<std::string, std::string> & getRepos() { return getRegistry()._repos; }

  /// returns the name() for a registered class
  template <typename T>
  static std::string getRegisteredName();

  ///@{ Don't allow creation through copy/move construction or assignment
  Registry(Registry const &) = delete;
  Registry & operator=(Registry const &) = delete;

  Registry(Registry &&) = delete;
  Registry & operator=(Registry &&) = delete;
  ///@}

private:
  /// Friends for unit testing
  ///@{
  friend class RegistryTest;
  friend class DataFileUtilsTest;
  FRIEND_TEST(RegistryTest, determineFilePath);
  FRIEND_TEST(RegistryTest, determineFilePathFailed);
  FRIEND_TEST(RegistryTest, appNameFromAppPath);
  FRIEND_TEST(RegistryTest, appNameFromAppPathFailed);
  ///@}

  Registry(){};

  /**
   * Manually set the data file paths.
   *
   * Used in unit testing.
   */
  static void setDataFilePaths(const std::map<std::string, std::string> & data_file_paths)
  {
    getRegistry()._data_file_paths = data_file_paths;
  }
  /**
   * Manually set the repos.
   *
   * Used in unit testing
   */
  static void setRepos(const std::map<std::string, std::string> & repos)
  {
    getRegistry()._repos = repos;
  }

  /// Internal helper for determing a root data file path (in-tree vs installed)
  static std::string determineDataFilePath(const std::string & name,
                                           const std::string & in_tree_path);

  /// Internal helper for getting an application name from its path, for example:
  /// /path/to/FooBarBazApp.C -> foo_bar_baz, for use in addDeprecatedAppDataFilePath
  static std::string appNameFromAppPath(const std::string & app_path);

  std::map<std::string, std::shared_ptr<RegistryEntryBase>> _name_to_entry;
  std::map<std::string, std::vector<std::shared_ptr<RegistryEntryBase>>> _per_label_objects;
  std::map<std::string, std::vector<std::shared_ptr<RegistryEntryBase>>> _per_label_actions;
  std::set<std::string> _known_labels;
  /// Data file registry; name -> in-tree path
  std::map<std::string, std::string> _data_file_paths;
  /// Repository name -> repository URL; used for mooseDocumentedError
  std::map<std::string, std::string> _repos;
  std::map<std::string, std::string> _type_to_classname;
};

template <typename T>
std::string
Registry::getRegisteredName()
{
  mooseDeprecated("Use Registry::getClassName() instead.");
  return getClassName<T>();
}

template <typename T>
std::unique_ptr<MooseObject>
RegistryEntry<T>::build(const InputParameters & parameters)
{
  if constexpr (std::is_base_of_v<MooseObject, T>)
    return std::make_unique<T>(parameters);
  mooseError("The object to be built is not derived from MooseObject.");
}

template <typename T>
std::shared_ptr<Action>
RegistryEntry<T>::buildAction(const InputParameters & parameters)
{
  if constexpr (!std::is_base_of_v<Action, T>)
    mooseError("The action to be built is not derived from Action.");
  else
    return std::make_shared<T>(parameters);
}

template <typename T>
InputParameters
RegistryEntry<T>::buildParameters()
{
  auto params = T::validParams();
  return params;
}

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
      Registry::addAction<classname>(                                                              \
          {app, #classname, "", task, nullptr, nullptr, nullptr, __FILE__, __LINE__, "", ""})

/// Add a MooseObject to the registry with the given app name/label.  classname is the (unquoted)
/// c++ class.  Each object/class should only be registered once.
#define registerMooseObject(app, classname)                                                        \
  static char combineNames(dummyvar_for_registering_obj_##classname, __COUNTER__) =                \
      Registry::add<classname>(                                                                    \
          {app, #classname, "", "", nullptr, nullptr, nullptr, __FILE__, __LINE__, "", ""})

#define registerADMooseObject(app, classname) registerMooseObject(app, classname)

/// Add a MooseObject to the registry with the given app name/label under an alternate alias/name
/// (quoted string) instead of the classname.
#define registerMooseObjectAliased(app, classname, alias)                                          \
  static char combineNames(dummyvar_for_registering_obj_##classname, __COUNTER__) =                \
      Registry::add<classname>(                                                                    \
          {app, #classname, alias, "", nullptr, nullptr, nullptr, __FILE__, __LINE__, "", ""})

/// Add a deprecated MooseObject to the registry with the given app name/label. time is the time
/// the object became/becomes deprecated in "mm/dd/yyyy HH:MM" format.
#define registerMooseObjectDeprecated(app, classname, time)                                        \
  static char combineNames(dummyvar_for_registering_obj_##classname, __COUNTER__) =                \
      Registry::add<classname>(                                                                    \
          {app, #classname, "", "", nullptr, nullptr, nullptr, __FILE__, __LINE__, time, ""})

#define registerADMooseObjectDeprecated(app, classname, time)                                      \
  registerMooseObjectDeprecated(app, classname, time)

/// add a deprecated MooseObject to the registry that has been replaced by another
/// object. time is the time the object became/becomes deprecated in "mm/dd/yyyy hh:mm" format.
#define registerMooseObjectReplaced(app, classname, time, replacement)                             \
  static char combineNames(dummyvar_for_registering_obj_##classname, __COUNTER__) =                \
      Registry::add<classname>({app,                                                               \
                                #classname,                                                        \
                                "",                                                                \
                                "",                                                                \
                                nullptr,                                                           \
                                nullptr,                                                           \
                                nullptr,                                                           \
                                __FILE__,                                                          \
                                __LINE__,                                                          \
                                time,                                                              \
                                #replacement})

/// add a deprecated MooseObject orig_class to the registry that has been replaced by another
/// object new_class with the same API. time is the time the object became/becomes deprecated in
/// "mm/dd/yyyy hh:mm" format.
/// A call to registerMooseObject is still required for the new class
#define registerMooseObjectRenamed(app, orig_class, time, new_class)                               \
  static char combineNames(dummyvar_for_registering_obj_##orig_class, __COUNTER__) =               \
      Registry::add<new_class>({app,                                                               \
                                #new_class,                                                        \
                                #orig_class,                                                       \
                                #orig_class,                                                       \
                                nullptr,                                                           \
                                nullptr,                                                           \
                                nullptr,                                                           \
                                __FILE__,                                                          \
                                __LINE__,                                                          \
                                time,                                                              \
                                #new_class})

#define registerADMooseObjectRenamed(app, orig_class, time, new_class)                             \
  registerMooseObjectRenamed(app, orig_class, time, new_class)

#define registerDataFilePath() Registry::addDataFilePath(__FILE__)

struct RegistryEntry;
class Factory;
class ActionFactory;
class MooseObject;
class Action;

using paramsPtr = InputParameters (*)();
using buildPtr = std::shared_ptr<MooseObject> (*)(const InputParameters & parameters);
using buildActionPtr = std::shared_ptr<Action> (*)(const InputParameters & parameters);

/// Holds details and meta-data info for a particular MooseObject or Action for use in the
/// registry.
struct RegistryEntry
{
  /// label (usually app name - e.g. "YourAnimalApp") that the object or action is associated with.
  std::string _label;
  /// name of the c++ class for the object.
  std::string _classname;
  /// an alternate name to register the object to factories under.  If unspecified, _classname is
  /// used.
  std::string _alias;
  /// name that the object will be registered to factories under.  If unspecified, _alias is used.
  std::string _name;
  /// function pointer for building instances of the MooseObject (if the entry is for an object).
  buildPtr _build_ptr;
  /// function pointer for building instances of the Action (if the entry is for an action).
  buildActionPtr _build_action_ptr;
  /// function pointer for building InputParameters objects for the object or action.
  paramsPtr _params_ptr;
  /// file path for the c++ file the object or action was added to the registry in.
  std::string _file;
  /// line number in the c++ file the object or action was added to the registry on.
  int _line;
  /// time in "mm/dd/yyyy HH:MM" format that the object is/becomes deprecated, blank otherwise.
  std::string _deprecated_time;
  /// class name for an object that replaces this object if deprecated, blank otherwise.
  std::string _replaced_by;
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
  static char add(const RegistryEntry & info)
  {
    RegistryEntry copy = info;
    copy._build_ptr = &build<T, MooseObject>;
    copy._params_ptr = &moose::internal::callValidParams<T>;
    addInner(copy);

    std::string name = info._name;
    if (name.empty())
      name = info._alias;
    if (name.empty())
      name = info._classname;
    getRegistry()._type_to_classname[typeid(T).name()] = name;

    return 0;
  }

  /// Adds information on an Action object to the registry.  The _build_ptr, _build_action_ptr, and
  /// _params_ptr objects of the info object should all be nullptr - these are set automatically by
  /// the addAction function itself using the templated type T.
  template <typename T>
  static char addAction(const RegistryEntry & info)
  {
    RegistryEntry copy = info;
    copy._build_action_ptr = &build<T, Action>;
    copy._params_ptr = &moose::internal::callValidParams<T>;
    addActionInner(copy);
    getRegistry()._type_to_classname[typeid(T).name()] = info._classname;
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

  /// register search paths for built-in data files
  static void addDataFilePath(const std::string & path);

  /// Returns a per-label keyed map of all MooseObjects in the registry.
  static const std::map<std::string, std::vector<RegistryEntry>> & allObjects()
  {
    return getRegistry()._per_label_objects;
  }
  /// Returns a per-label keyed map of all Actions in the registry.
  static const std::map<std::string, std::vector<RegistryEntry>> & allActions()
  {
    return getRegistry()._per_label_actions;
  }

  static RegistryEntry & objData(const std::string & name);

  /**
   * \returns true if an object with the given name is registered
   */
  static bool isRegisteredObj(const std::string & name)
  {
    return getRegistry()._name_to_entry.count(name);
  }

  /// Returns a vector of all registered data file paths
  static const std::vector<std::string> & getDataFilePaths()
  {
    return getRegistry()._data_file_paths;
  }

  ///@{ Don't allow creation through copy/move construction or assignment
  Registry(Registry const &) = delete;
  Registry & operator=(Registry const &) = delete;

  Registry(Registry &&) = delete;
  Registry & operator=(Registry &&) = delete;
  ///@}

private:
  Registry(){};

  static void addInner(const RegistryEntry & info);
  static void addActionInner(const RegistryEntry & info);

  template <typename T, typename ToType>
  static std::shared_ptr<ToType> build(const InputParameters & parameters)
  {
    return std::make_shared<T>(parameters);
  }

  std::map<std::string, RegistryEntry> _name_to_entry;
  std::map<std::string, std::vector<RegistryEntry>> _per_label_objects;
  std::map<std::string, std::vector<RegistryEntry>> _per_label_actions;
  std::set<std::string> _known_labels;
  std::vector<std::string> _data_file_paths;
  std::map<std::string, std::string> _type_to_classname;
};

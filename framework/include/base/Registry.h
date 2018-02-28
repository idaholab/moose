//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef REGISTRY_H
#define REGISTRY_H

#include <string>
#include <vector>
#include <set>
#include <map>

#define combineNames1(X, Y) X##Y
#define combineNames(X, Y) combineNames1(X, Y)

#define registerMooseAction(app, classname, task)                                                  \
  static char combineNames(dummyvar_for_registering_action_##classname, __LINE__) =                \
      Registry::addAction<classname>(                                                              \
          {app, #classname, "", task, nullptr, nullptr, nullptr, __FILE__, __LINE__, "", ""})

#define registerMooseObject(app, classname)                                                        \
  static char combineNames(dummyvar_for_registering_obj_##classname, __LINE__) =                   \
      Registry::add<classname>(                                                                    \
          {app, #classname, "", "", nullptr, nullptr, nullptr, __FILE__, __LINE__, "", ""})

#define registerMooseObjectAliased(app, classname, alias)                                          \
  static char combineNames(dummyvar_for_registering_obj_##classname, __LINE__) =                   \
      Registry::add<classname>(                                                                    \
          {app, #classname, alias, "", nullptr, nullptr, nullptr, __FILE__, __LINE__, "", ""})

#define registerMooseObjectDeprecated(app, classname, time)                                        \
  static char combineNames(dummyvar_for_registering_obj_##classname, __LINE__) =                   \
      Registry::add<classname>(                                                                    \
          {app, #classname, "", "", nullptr, nullptr, nullptr, __FILE__, __LINE__, time, ""})

#define registerMooseObjectReplaced(app, classname, time, replacement)                             \
  static char combineNames(dummyvar_for_registering_obj_##classname, __LINE__) =                   \
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

#define registerMooseObjectRenamed(app, orig_class, time, new_class)                               \
  static char combineNames(dummyvar_for_registering_obj_##orig_name, __LINE__) =                   \
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

struct RegistryEntry;
class Factory;
class ActionFactory;
class InputParameters;
class MooseObject;
class Action;

using paramsPtr = InputParameters (*)();
using buildPtr = std::shared_ptr<MooseObject> (*)(const InputParameters & parameters);
using buildActionPtr = std::shared_ptr<Action> (*)(const InputParameters & parameters);

struct RegistryEntry
{
  std::string _label;
  std::string _classname;
  std::string _alias;
  std::string _name;
  buildPtr _build_ptr;
  buildActionPtr _build_action_ptr;
  paramsPtr _params_ptr;
  std::string _file;
  int _line;
  std::string _deprecated_time;
  std::string _replaced_by;
};

template <class T>
std::shared_ptr<MooseObject>
buildObj(const InputParameters & parameters)
{
  return std::make_shared<T>(parameters);
}

template <class T>
std::shared_ptr<Action>
buildAct(const InputParameters & parameters)
{
  return std::make_shared<T>(parameters);
}

class Registry
{
public:
  template <typename T>
  static char add(const RegistryEntry & info)
  {
    RegistryEntry copy = info;
    copy._build_ptr = &buildObj<T>;
    copy._params_ptr = &validParams<T>;
    addInner(copy);
    return 0;
  }

  template <typename T>
  static char addAction(const RegistryEntry & info)
  {
    RegistryEntry copy = info;
    copy._build_action_ptr = &buildAct<T>;
    copy._params_ptr = &validParams<T>;
    addActionInner(copy);
    return 0;
  }

  static void registerObjectsTo(Factory & f, const std::set<std::string> & labels);
  static void registerActionsTo(ActionFactory & f, const std::set<std::string> & labels);
  static void checkLabels(const std::set<std::string> & known_labels = {});
  static void addKnownLabel(const std::string & label);
  static const std::map<std::string, std::vector<RegistryEntry>> & allObjects();
  static const std::map<std::string, std::vector<RegistryEntry>> & allActions();

private:
  static void addInner(const RegistryEntry & info);
  static void addActionInner(const RegistryEntry & info);

  std::map<std::string, std::vector<RegistryEntry>> _per_label_objects;
  std::map<std::string, std::vector<RegistryEntry>> _per_label_actions;
  std::set<std::string> _known_labels;
};

#endif

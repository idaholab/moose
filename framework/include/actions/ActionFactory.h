//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <vector>
#include <map>
#include <set>

#include "Action.h" // Technically required for std::shared_ptr<Action>(Action*) constructor
#include "InputParameters.h"
#include "FileLineInfo.h"

/**
 * Macros
 */
#define stringifyName(name) #name
#define registerAction(tplt, action)                                                               \
  action_factory.reg<tplt>(stringifyName(tplt), action, __FILE__, __LINE__)

#define registerSyntax(action, action_syntax)                                                      \
  syntax.registerActionSyntax(action, action_syntax, "", __FILE__, __LINE__)
#define registerSyntaxTask(action, action_syntax, task)                                            \
  syntax.registerActionSyntax(action, action_syntax, task, __FILE__, __LINE__)
#define registerTask(name, is_required) syntax.registerTaskName(name, is_required)
#define registerMooseObjectTask(name, moose_system, is_required)                                   \
  syntax.registerTaskName(name, stringifyName(moose_system), is_required)
#define appendMooseObjectTask(name, moose_system)                                                  \
  syntax.appendTaskName(name, stringifyName(moose_system))
#define addTaskDependency(action, depends_on) syntax.addDependency(action, depends_on)

// Forward Declaration
class MooseApp;

/**
 * Typedef for function to build objects
 */
typedef std::shared_ptr<Action> (*buildActionPtr)(const InputParameters & parameters);

/**
 * Typedef for validParams
 */
typedef InputParameters (*paramsActionPtr)();

/**
 * Specialized factory for generic Action System objects
 */
class ActionFactory
{
public:
  ActionFactory(MooseApp & app);

  virtual ~ActionFactory();

  MooseApp & app() { return _app; }

  template <typename T>
  void reg(const std::string & name,
           const std::string & task,
           const std::string & file = "",
           int line = -1)
  {
    reg(name, task, &buildAction<T>, &moose::internal::callValidParams<T>, file, line);
  }

  void reg(const std::string & name,
           const std::string & task,
           buildActionPtr obj_builder,
           paramsActionPtr ref_params,
           const std::string & file = "",
           int line = -1);

  /**
   * Gets file and line information where an action was registered.
   * @param name Action name
   * @param task task name
   * @return A FileLineInfo associated with the name/task pair
   */
  FileLineInfo getLineInfo(const std::string & name, const std::string & task) const;

  std::string getTaskName(const std::string & action);

  std::shared_ptr<Action>
  create(const std::string & action, const std::string & action_name, InputParameters & parameters);

  InputParameters getValidParams(const std::string & name);

  class BuildInfo
  {
  public:
    buildActionPtr _build_pointer;
    paramsActionPtr _params_pointer;
    std::string _task;
  };

  /// Typedef for registered Action iterator
  typedef std::multimap<std::string, BuildInfo>::iterator iterator;
  typedef std::multimap<std::string, BuildInfo>::const_iterator const_iterator;

  iterator begin();
  const_iterator begin() const;

  iterator end();
  const_iterator end() const;

  std::pair<std::multimap<std::string, std::string>::const_iterator,
            std::multimap<std::string, std::string>::const_iterator>
  getActionsByTask(const std::string & task) const;

  std::set<std::string> getTasksByAction(const std::string & action) const;

  /**
   * Whether or not a task with the name \p task is registered.
   */
  bool isRegisteredTask(const std::string & task) const { return _tasks.count(task); }

private:
  template <class T>
  static std::shared_ptr<Action> buildAction(const InputParameters & parameters)
  {
    return std::make_shared<T>(parameters);
  }

  MooseApp & _app;

  std::multimap<std::string, BuildInfo> _name_to_build_info;

  FileLineInfoMap _name_to_line;
  std::multimap<std::string, std::string> _task_to_action_map;

  /// set<objectname, task> used to track if an object previously added is being added again
  std::set<std::pair<std::string, std::string>> _current_objs;

  /// The registered tasks
  std::set<std::string> _tasks;
};

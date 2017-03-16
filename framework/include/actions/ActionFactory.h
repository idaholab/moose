/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef ACTIONFACTORY_H
#define ACTIONFACTORY_H

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
typedef std::shared_ptr<Action> (*buildActionPtr)(InputParameters parameters);

/**
 * Typedef for validParams
 */
typedef InputParameters (*paramsActionPtr)();

/**
 * Build an object of type T
 */
template <class T>
std::shared_ptr<Action>
buildAction(InputParameters parameters)
{
  return std::shared_ptr<Action>(new T(parameters));
}

/**
 * Specialized factory for generic Action System objects
 */
class ActionFactory
{
public:
  ActionFactory(MooseApp & app);

  virtual ~ActionFactory();

  template <typename T>
  void reg(const std::string & name,
           const std::string & task,
           const std::string & file = "",
           int line = -1)
  {
    BuildInfo build_info;
    build_info._build_pointer = &buildAction<T>;
    build_info._params_pointer = &validParams<T>;
    build_info._task = task;
    build_info._unique_id = _unique_id++;
    _name_to_build_info.insert(std::make_pair(name, build_info));
    _task_to_action_map.insert(std::make_pair(task, name));
    _name_to_line.addInfo(name, task, file, line);
  }

  /**
   * Gets file and line information where an action was registered.
   * @param name Action name
   * @param task task name
   * @return A FileLineInfo associated with the name/task pair
   */
  FileLineInfo getLineInfo(const std::string & name, const std::string & task) const;

  std::string getTaskName(const std::string & action);

  std::shared_ptr<Action>
  create(const std::string & action, const std::string & action_name, InputParameters parameters);

  InputParameters getValidParams(const std::string & name);

  class BuildInfo
  {
  public:
    buildActionPtr _build_pointer;
    paramsActionPtr _params_pointer;
    std::string _task;
    unsigned int _unique_id;
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

protected:
  MooseApp & _app;

  std::multimap<std::string, BuildInfo> _name_to_build_info;

  FileLineInfoMap _name_to_line;
  std::multimap<std::string, std::string> _task_to_action_map;

  // TODO: I don't think we need this anymore
  static unsigned int _unique_id; ///< Unique ID for identifying multiple registrations
};

#endif /* ACTIONFACTORY_H */

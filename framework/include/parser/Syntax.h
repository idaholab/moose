//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SYNTAX_H
#define SYNTAX_H

#include <string>
#include <map>
#include "DependencyResolver.h"
#include "FileLineInfo.h"

/**
 * Holding syntax for parsing input files
 */
class Syntax
{
public:
  struct ActionInfo
  {
    std::string _action;
    std::string _task;
  };

  Syntax();

  /**
   * Method to register a new task. Tasks are short verbs (strings) that describe a particular point
   * in the simulation setup phase.
   * @param task The task (verb) to be registered with MOOSE.
   * @param is_required indicates whether the task is required for a sucessful simulation.
   */
  void registerTaskName(const std::string & task, bool is_required);

  /**
   * Method to register a new task (see overload method with same name). This version also accepts
   * a string representing a pluggable MOOSE system. When objects are created through a task, the
   * MOOSE system is checked if it exists to make sure it's "allowed" to create those types of
   * objects.
   */
  void registerTaskName(const std::string & task,
                        const std::string & moose_object_type,
                        bool is_required);

  /**
   * Method to associate another "allowed" pluggable MOOSE system to an existing registered task.
   * Each object created during a task is checked against associated systems.
   */
  void appendTaskName(const std::string & task, const std::string & moose_object_type);

  /**
   * Method to insert a new task in the list of existing tasks at the specified location.
   */
  void addDependency(const std::string & task, const std::string & pre_req);

  /**
   * Adds all dependencies in a single call. The string is split on parenthesis and each task listed
   * within the parenthesis is given equal precedence.
   */
  void addDependencySets(const std::string & action_sets);

  /**
   * Clears all tasks from the system object.
   */
  void clearTaskDependencies();

  /**
   * Get a list of serialized tasks in a correct dependency order. The order my be more ordered than
   * specified.
   */
  const std::vector<std::string> & getSortedTask();

  /**
   * Get a list of serialized tasks in a correct dependency order. The vector of sets return type
   * assures that tasks with equal precedence appear in a single set.
   */
  const std::vector<std::vector<std::string>> & getSortedTaskSet();

  /**
   * Returns a Boolean indicating whether or not a task is registered with the syntax object.
   */
  bool hasTask(const std::string & task);

  /**
   * Returns a Boolean indicating whether the specified task is required.
   */
  bool isActionRequired(const std::string & task);

  /**
   * Registration function for associating Moose Actions with syntax.
   */
  void registerActionSyntax(const std::string & action,
                            const std::string & syntax,
                            const std::string & task = "",
                            const std::string & file = "",
                            int line = -1);

  /**
   *  Registration function that replaces existing Moose Actions with a completely new action
   *  Note: This function will remove all actions associated with this piece of syntax _NOT_ just
   *        a single match of some kind
   */
  void replaceActionSyntax(const std::string & action,
                           const std::string & syntax,
                           const std::string & task,
                           const std::string & file = "",
                           int line = -1);

  /**
   * Register a type with a block. For example, associate FunctionName with the Functions block.
   * @param syntax The target syntax to associate the type with
   * @param type The name of the type to associate with the syntax
   */
  void registerSyntaxType(const std::string & syntax, const std::string & type);

  /**
   * Get a multimap of registered associations of syntax with type.
   */
  const std::multimap<std::string, std::string> & getAssociatedTypes() const;

  /**
   * This method deprecates previously registered syntax. You should use the exact form that you
   * want deprecated in the passed in parameter.
   */
  void deprecateActionSyntax(const std::string & syntax);

  /**
   * Returns a Boolean indicating whether the syntax has been deprecated through a call to
   * deprecateActionSyntax.
   */
  bool isDeprecatedSyntax(const std::string & syntax) const;

  /**
   * Retrieve the syntax associated with the passed in action type string. If a task string is also
   * passed in, only syntax associated with that action+task combo will be returned.
   */
  std::vector<std::string> getSyntaxByAction(const std::string & action,
                                             const std::string & task = "");

  /**
   * Method for determining whether a piece of syntax is associated with an Action
   * TODO: I need a better name
   */
  std::string isAssociated(const std::string & real_id, bool * is_parent);

  /**
   * Returns a pair of multimap iterators to all the ActionInfo objects associated with a given
   * piece of syntax.
   */
  std::pair<std::multimap<std::string, ActionInfo>::const_iterator,
            std::multimap<std::string, ActionInfo>::const_iterator>
  getActions(const std::string & syntax) const;

  /**
   * Return all Syntax to Action associations.
   */
  const std::multimap<std::string, ActionInfo> & getAssociatedActions() const;

  /**
   * Returns a Boolean indicating whether a task is associated with on of the MOOSE pluggable
   * systems (BASE CLASSES). See "registerMooseObjectTask" macro in Moose.C. This information can be
   * used to determine whether certain objects may be safely built during the specified task.
   */
  bool verifyMooseObjectTask(const std::string & base, const std::string & task) const;

  /**
   * Gets the file and line where the syntax/action/task combo was registered.
   * @param syntax Syntax name
   * @param action Action name
   * @param task Task name
   * @return A FileLineInfo associated with the syntax/action/task triplet
   */
  FileLineInfo getLineInfo(const std::string & syntax,
                           const std::string & action,
                           const std::string & task) const;

protected:
  /// The list of registered tasks and a flag indicating whether or not they are required
  std::map<std::string, bool> _registered_tasks;

  /// The list of Moose system objects to tasks.  This map indicates which tasks are allowed to build certain MooseObjects.
  std::multimap<std::string, std::string> _moose_systems_to_tasks;

  /// The dependency resolver
  DependencyResolver<std::string> _tasks;

  /// The syntax object to ActionInfo (Action+task) associations
  std::multimap<std::string, ActionInfo> _syntax_to_actions;

  /// The ActionInfo (Action+task) to syntax associations (built only when needed)
  /// Action -> (Syntax, Task)
  std::multimap<std::string, std::pair<std::string, std::string>> _actions_to_syntax;

  /// Syntax/Type association
  std::multimap<std::string, std::string> _associated_types;

  /// Boolean indicating whether the _actions_to_syntax map is built and valid and synced
  bool _actions_to_syntax_valid;

  /// The set of deprecated syntax items
  std::set<std::string> _deprecated_syntax;

  FileLineInfoMap _syntax_to_line;
};

#endif // MOOSESYNTAX_H

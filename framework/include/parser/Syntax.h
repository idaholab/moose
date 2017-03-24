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

public:
  Syntax();

  void registerTaskName(std::string task, bool is_required);
  void registerTaskName(std::string task, std::string moose_object_type, bool is_required);
  void appendTaskName(std::string task, std::string moose_object_type);

  void addDependency(std::string task, std::string pre_req);
  void addDependencySets(const std::string & action_sets);

  const std::vector<std::string> & getSortedTask();
  const std::vector<std::vector<std::string>> & getSortedTaskSet();

  bool hasTask(const std::string & task);

  bool isActionRequired(const std::string & task);

  // Registration function for associating Moose Actions with syntax
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
   *  Registration a type with a block. For example, associate FunctionName with the Functions block
   * @param syntax The target syntax to associate the type with
   * @param type The name of the type to associate with the syntax
   */
  void registerSyntaxType(const std::string & syntax, const std::string & type);

  /**
   * Get a multimap of registered associations of syntax with type.
   */
  const std::multimap<std::string, std::string> & getAssociatedTypes() const
  {
    return _associated_types;
  }

  /**
   * This method deprecates previously registered syntax. You should use the exact form that you
   * want deprecated
   * in the passed in parameter.
   */
  void deprecateActionSyntax(const std::string & syntax);

  /// Returns a Boolean indicating whether the syntax has been deprecated through a call to deprecateActionSyntax
  bool isDeprecatedSyntax(const std::string & syntax) const;

  // Retrieve the Syntax associated with the passed Action and task
  std::string getSyntaxByAction(const std::string & action, const std::string & task);

  /**
   * Method for determining whether a piece of syntax is associated with an Action
   * TODO: I need a better name
   */
  std::string isAssociated(const std::string & real_id, bool * is_parent);

  std::pair<std::multimap<std::string, ActionInfo>::iterator,
            std::multimap<std::string, ActionInfo>::iterator>
  getActions(const std::string & name);

  const std::multimap<std::string, ActionInfo> & getAssociatedActions() const
  {
    return _associated_actions;
  }

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

  /// Actions/Syntax association
  std::multimap<std::string, ActionInfo> _associated_actions;

  /// Syntax/Type association
  std::multimap<std::string, std::string> _associated_types;

  std::set<std::string> _deprecated_syntax;
  FileLineInfoMap _syntax_to_line;
};

#endif // MOOSESYNTAX_H

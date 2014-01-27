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
  const std::vector<std::set<std::string> > & getSortedTaskSet();

  bool hasTask(const std::string & task);

  bool isActionRequired(const std::string & task);

  // Registration function for associating Moose Actions with syntax
  void registerActionSyntax(const std::string & action, const std::string & syntax,
                            const std::string & task = "");

  /**
   *  Registration function that replaces existing Moose Actions with a completely new action
   *  Note: This function will remove all actions associated with this piece of syntax _NOT_ just
   *        a single match of some kind
   */
  void replaceActionSyntax(const std::string & action, const std::string & syntax, const std::string & task);

  // Retrieve the Syntax associated with the passed Action and task
  std::string getSyntaxByAction(const std::string & action, const std::string & task);

  /**
   * Method for determining whether a piece of syntax is associated with an Action
   * TODO: I need a better name
   */
  std::string isAssociated(const std::string & real_id, bool * is_parent);

  std::pair<std::multimap<std::string, ActionInfo>::iterator, std::multimap<std::string, ActionInfo>::iterator>
  getActions(const std::string & name);

  const std::multimap<std::string, ActionInfo> & getAssociatedActions() const { return _associated_actions; }

  bool verifyMooseObjectTask(const std::string & base, const std::string & task) const;

protected:
  /// The list of registered tasks and a flag indicating whether or not they are required
  std::map<std::string, bool> _registered_tasks;

  /// The list of Moose system objects to tasks.  This map indicates which tasks are allowed to build certain MooseObjects.
  std::multimap<std::string, std::string> _moose_systems_to_tasks;

  /// The dependency resolver
  DependencyResolver<std::string> _tasks;

  /// Actions/Syntax association
  std::multimap<std::string, ActionInfo> _associated_actions;
};


#endif // MOOSESYNTAX_H

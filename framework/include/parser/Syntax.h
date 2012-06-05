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
    std::string _action_name;
  };

public:
  Syntax();

  void registerName(std::string action, bool is_required);
  void addDependency(std::string action, std::string pre_req);
  void addDependencySets(const std::string & action_sets);

  const std::vector<std::string> & getSortedActionName();
  const std::vector<std::set<std::string> > & getSortedActionNameSet();

  bool hasActionName(const std::string & action_name);

  bool isActionRequired(const std::string & action_name);

  // Registration function for associating Moose Actions with syntax
  void registerActionSyntax(const std::string & action, const std::string & syntax,
                            const std::string & action_name = "");

  /**
   *  Registration function that replaces existing Moose Actions with a completely new action
   *  Note: This function will remove all actions associated with this piece of syntax _NOT_ just
   *        a single match of some kind
   */
  void replaceActionSyntax(const std::string & action, const std::string & syntax, const std::string & action_name);

  // Retrieve the Syntax associated with the passed Action and action_name
  std::string getSyntaxByAction(const std::string & action, const std::string & action_name);

  /**
   * Method for determining whether a piece of syntax is associated with an Action
   * TODO: I need a better name
   */
  std::string isAssociated(const std::string & real_id, bool * is_parent);

  std::pair<std::multimap<std::string, ActionInfo>::iterator, std::multimap<std::string, ActionInfo>::iterator>
  getActions(const std::string & name);

  std::multimap<std::string, ActionInfo> & getAssociatedActions() { return _associated_actions; }

protected:
  /// The list of registered actions and a flag indicating whether or not they are required
  std::map<std::string, bool> _registered_actions;
  /// The dependency resolver
  DependencyResolver<std::string> _actions;
  /// Actions/Syntax association
  std::multimap<std::string, ActionInfo> _associated_actions;

};


#endif // MOOSESYNTAX_H

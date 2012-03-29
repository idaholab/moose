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

#ifndef MOOSESYNTAX_H
#define MOOSESYNTAX_H

#include <string>
#include <map>

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

  /// Actions/Syntax association
  std::multimap<std::string, ActionInfo> _associated_actions;

};


namespace Moose
{
void associateSyntax();

extern Syntax syntax;
}

#endif // MOOSESYNTAX_H

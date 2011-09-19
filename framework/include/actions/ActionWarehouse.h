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

#ifndef ACTIONWAREHOUSE_H
#define ACTIONWAREHOUSE_H

#include <string>
#include <set>
#include <map>
#include <ostream>

#include "DependencyResolver.h"
#include "Action.h"
#include "EmptyAction.h"

/// Typedef to hide implementation details
typedef std::vector<Action *>::iterator ActionIterator;

class ActionWarehouse
{
public:
  ActionWarehouse();
  ~ActionWarehouse();

  void clear();

  void setParserPointer(Parser * p_ptr) { _parser_ptr = p_ptr; }
  void registerName(std::string action, bool is_required);
  void addDependency(std::string action, std::string pre_req);
  void addDependencySets(const std::string & action_sets);
  void addActionBlock(Action * blk);

  /**
   * This method checks the actions stored in the warehouse against the list of required resgistered
   * actions to see if all of them have been satisified.  It should be called before running
   * a MOOSE problem
   */
  void checkUnsatisfiedActions() const;

  void printActionDependencySets();

  ActionIterator actionBlocksWithActionBegin(const std::string & action_name);
  ActionIterator actionBlocksWithActionEnd(const std::string & action_name);

  /**
   * This method loops over all actions in the warehouse and executes them.  Meta-actions
   * may add new actions to the warehouse on the fly and they will still be executed in order
   */
  void executeAllActions();

  /**
   * This method executes only the actions in the warehouse that satisfy the action_name
   * passed in
   */
  void executeActionsWithAction(const std::string & name);

  /**
   * Iterator class for returning the Actions stored in this warehouse in order.
   * This class is necessary to support the Meta-action capability supported by the
   * ActionWarehouse.  Meta-Actions can add new Actions to the warehouse while
   * maintaining proper order and a valid iterator.
   */
  class iterator
  {
  public:
    iterator(ActionWarehouse & act_wh, bool end=false);
    bool operator==(const iterator &rhs) const;
    bool operator!=(const iterator &rhs) const;
    iterator & operator++();
    Action * operator*();

  private:
    ActionWarehouse & _act_wh;
    bool _first;
    bool _end;
    std::vector<std::string>::iterator _i;
    std::vector<Action *>::iterator _j;
  };
  friend class iterator;

  iterator begin();
  iterator end();

  void printInputFile(std::ostream & stream);

  void showActions(bool state = true) { _show_actions = state; }

private:
  void buildBuildableActions(const std::string &action_name);

  /// The list of registered actions and a flag indicating whether or not they are required
  std::map<std::string, bool> _registered_actions;

  /// Pointers to the actual parsed input file blocks
  std::map<std::string, std::vector<Action *> > _action_blocks;

  /// The dependency resolver
  DependencyResolver<std::string> _actions;

  /// The container that holds the sorted action names from the DependencyResolver
  std::vector<std::string> _ordered_names;

  /// Use to store the current list of unsatisfied dependencies
  std::set<std::string> _unsatisfied_dependencies;

  /**
   *  Flag to indicate whether or not there is an active iterator on this class.
   *  There can only be a single active iterator because of the potential for
   *  meta Actions to add new Actions into the warehouse on the fly
   */
  bool _generator_valid;

  /// Pointer to the active parser for this Warehouse instance
  Parser * _parser_ptr;

  // DEBUGGING
  bool _show_actions;
};

#endif // ACTIONWAREHOUSE_H

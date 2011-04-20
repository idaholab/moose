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
  
  /// Generators to ordered Actions in this Warehouse
  // TODO: Right now all Actions require a Parser pointer when setting up the problem.
  //       In order to build Actions on the fly inside of the factory we'll need this
  //       pointer when the parser iterates over the Actions.  We might be able
  //       to make this cleaner later.
  /// This method initiates the generator and returns the first ordered action
  Action * allActionsBegin(Parser * p_ptr);
  /// This method is a generator function that returns the next ordered action
  Action * allActionsNext(Parser * p_ptr, bool very_first = false);
  
  ActionIterator inputFileActionsBegin();
  ActionIterator inputFileActionsEnd();

private:
  void buildBuildableActions(Parser * p_ptr);
  
  /// The list of registered actions and a flag indicating whether or not they are required
  std::map<std::string, bool> _registered_actions;

  /// Pointers to the actual parsed input file blocks
  std::map<std::string, std::vector<Action *> > _action_blocks;

  /// The dependency resolver
  DependencyResolver<std::string> _actions;

  /// The vector of ordered actions out of the dependency resolver
  std::vector<Action *> _ordered_actions;

  std::vector<std::string> _ordered_names;
  
  /// Used to store the action name for the current active action Block iterator
  std::string _curr_action_name;

  Action * _empty_action;

  /// Use to store the current list of unsatisfied dependencies
  std::set<std::string> _unsatisfied_dependencies;

  std::vector<std::string>::iterator _i;
  std::vector<Action *>::iterator _j;
  bool _generator_valid;

  // Functor for sorting input file syntax 
  class InputFileSort 
  {  
  public: 
    InputFileSort(); 
    bool operator() (Action *a, Action *b); 
    
  private: 
    std::vector<std::string> _o; 
  }; 
};

#endif // ACTIONWAREHOUSE_H

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
  void addActionBlock(Action * blk);

  ActionIterator actionBlocksWithActionBegin(const std::string & action_name);
  ActionIterator actionBlocksWithActionEnd(const std::string & action_name);
  
  /// Iterators to ordered Actions
  // TODO: Right now all Actions require a Parser pointer when setting up the problem.
  //       In order to build Actions on the fly inside of the factory we'll need this
  //       pointer when the parser iterates over the Actions.  We might be able
  //       to make this cleaner later
  ActionIterator allActionsBegin(Parser * p_ptr);
  ActionIterator allActionsEnd();

  ActionIterator inputFileActionsBegin();
  ActionIterator inputFileActionsEnd();

private:
  /// The list of registered actions and a flag indicating whether or not they are required
  std::map<std::string, bool> _registered_actions;

  /// Pointers to the actual parsed input file blocks
  std::map<std::string, std::vector<Action *> > _action_blocks;

  /// The dependency resolver
  DependencyResolver<std::string> _actions;

  /// The vector of ordered actions out of the dependency resolver
  std::vector<Action *> _ordered_actions;
  
  /// Used to store the action name for the current active action Block iterator
  std::string _curr_action_name;

  EmptyAction *_empty_action;
};

#endif // ACTIONWAREHOUSE_H

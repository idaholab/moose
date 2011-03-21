#ifndef ACTIONWAREHOUSE_H
#define ACTIONWAREHOUSE_H

#include <string>
#include <set>
#include <map>

#include "DependencyResolver.h"
#include "Action.h"

/// Typedef to hide implementation details
typedef std::vector<Action *>::iterator ActionIterator;

class ActionWarehouse
{
public:
  ActionWarehouse();

  void registerName(std::string action, bool is_required);
  void addDependency(std::string action, std::string pre_req);
  void addActionBlock(Action * blk);

  /// Iterators to ordered Actions

  // TODO: Right now all Actions require a Parser pointer when setting up the problem.
  //       In order to build Actions on the fly inside of the factory we'll need this
  //       pointer when the parser iterates over the Actions.  We might be able
  //       to make this cleaner later
  ActionIterator allActionsBegin(Parser * p_ptr);
  ActionIterator allActionsEnd();

private:
  /// The list of registered actions and a flag indicating whether or not they are required
  std::map<std::string, bool> _registered_actions;

  /// Pointers to the actual parsed input file blocks
  std::map<std::string, std::vector<Action *> > _action_blocks;

  /// The dependency resolver
  DependencyResolver<std::string> _actions;

  /// The vector of ordered actions out of the dependency resolver
  std::vector<Action *> _ordered_actions;
};

#endif // ACTIONWAREHOUSE_H

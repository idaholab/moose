#ifndef ACTIONWAREHOUSE_H
#define ACTIONWAREHOUSE_H

#include <string>
#include <set>
#include <map>

#include "DependencyResolver.h"
#include "Action.h"

/// Typedef to hide implementation details
typedef std::vector<Action *>::iterator ActionIterator;

namespace Moose
{

class ActionWarehouse
{
public:
  ActionWarehouse();

  void addAction(std::string action);
  void addAction(std::string action, std::string pre_req, bool is_required=false);
  void addActionBlock(Action * blk);

  /// Iterators to ordered Actions
  ActionIterator allActionsBegin();
  ActionIterator allActionsEnd();

private:
  /// This memeber holds the list of registered actions for error checking
  std::set<std::string> _registered_actions;

  /// This member holds pointers to the actual parsed input file blocks
  std::map<std::string, std::vector<Action *> > _action_blocks;

  /// The dependency resolver
  DependencyResolver<std::string> _actions;

  /// This memeber holds the ordered Actions
  std::vector<Action *> _ordered_actions;
};

} // Namespace Moose


#endif // ACTIONWAREHOUSE_H

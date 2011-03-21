#include "ActionWarehouse.h"

ActionWarehouse::ActionWarehouse() :
    _ordered_actions(NULL)
{
}

void
ActionWarehouse::addAction(std::string action)
{
  _actions.addItem(action);
  _registered_actions.insert(action);
}

void
ActionWarehouse::addAction(std::string action, std::string pre_req, bool is_required)
{
  _actions.insertDependency(action, pre_req);
  _registered_actions.insert(action);
}

void
ActionWarehouse::addActionBlock(Action * blk)
{
  std::string action_name = blk->getAction();
  if (_registered_actions.find(action_name) == _registered_actions.end())
    mooseError("A " << action_name << " is not a registered action name");
  
  _action_blocks[action_name].push_back(blk);
}

ActionIterator
ActionWarehouse::allActionsBegin()
{
  // Clear the current Action vector so that we can fill it up with new ordered actions
  _ordered_actions.clear();

#ifdef DEBUG
  std::cerr << "Ordered Actions:\n";
#endif
  
  std::vector<std::string> _ordered_names = _actions.getSortedValues();
  for (std::vector<std::string>::iterator i = _ordered_names.begin(); i != _ordered_names.end(); ++i)
  {
#ifdef DEBUG
    std::cerr << *i << "\n";

    for (std::vector<Action *>::iterator j = _action_blocks[*i].begin(); j != _action_blocks[*i].end(); ++j)
      std::cerr << "\t" << (*j)->getAction() << "\n";
                                                                  
#endif
    // append all the Actions that satisfy this "action" onto the end of the ordered_vector
    _ordered_actions.insert(_ordered_actions.end(), _action_blocks[*i].begin(), _action_blocks[*i].end());
  }
#ifdef DEBUG
  std::cerr << "\n";
#endif
  
  
  return _ordered_actions.begin();
}

ActionIterator
ActionWarehouse::allActionsEnd()
{
  return _ordered_actions.end();
}

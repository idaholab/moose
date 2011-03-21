#include "ActionWarehouse.h"
#include "ActionFactory.h"
#include "Parser.h"

ActionWarehouse::ActionWarehouse() :
  _ordered_actions(NULL),
  _empty_action(NULL)
{
}

ActionWarehouse::~ActionWarehouse()
{
  if (_empty_action)
    delete _empty_action;
}

void
ActionWarehouse::clear()
{
  _action_blocks.clear();
}

void
ActionWarehouse::registerName(std::string action, bool is_required)
{
  _actions.addItem(action);
  _registered_actions[action] = is_required;
}

void
ActionWarehouse::addDependency(std::string action, std::string pre_req)
{
  if (_registered_actions.find(action) == _registered_actions.end())
    mooseError("A " << action << " is not a registered action name");
  
  _actions.insertDependency(action, pre_req);
}

void
ActionWarehouse::addDependencySets(const std::string & action_sets)
{
  std::vector<std::string> sets, prev_names, action_names;
  Parser::tokenize(action_sets, sets, "()");

  for (unsigned int i=0; i<sets.size(); ++i)
  {
    action_names.clear();
    Parser::tokenize(sets[i], action_names, ", ");
    for (unsigned int j=0; j<action_names.size(); ++j)
    {
      // Each line should depend on each item in the previous line
      for (unsigned int k=0; k<prev_names.size(); ++k)
        addDependency(action_names[j], prev_names[k]);
    }
    // Copy the the current items to the previous items for the next iteration
    std::swap(action_names, prev_names);
  }
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
ActionWarehouse::actionBlocksWithActionBegin(const std::string & action_name)
{
  return _action_blocks[action_name].begin();
}

ActionIterator
ActionWarehouse::actionBlocksWithActionEnd(const std::string & action_name)
{
  return _action_blocks[action_name].end();
}

ActionIterator
ActionWarehouse::inputFileActionsBegin()
{
  if (!_empty_action)
    _empty_action = new EmptyAction("", validParams<Action>());
  
  std::map<std::string, std::vector<Action *> >::iterator iter;
  
  // We'll use a map to reorder for us
  std::map<std::string, Action *> input_file_blocks;
  std::map<std::string, Action *>::iterator i;
  
  for (iter = _action_blocks.begin(); iter != _action_blocks.end(); ++iter)
    for (std::vector<Action *>::iterator j = iter->second.begin(); j != iter->second.end(); ++j)
      input_file_blocks[(*j)->name()] = *j;

  _ordered_actions.clear();
  for (i = input_file_blocks.begin(); i != input_file_blocks.end(); ++i)
  {
    std::cerr << i->second->name() << '\n';
    _ordered_actions.push_back(i->second);
  }
  
  // We'll push one more "empty" action onto the end so that when we print the input syntax
  // everything will get closed off without any odd tail calls.
  _ordered_actions.push_back(_empty_action);
  
  return _ordered_actions.begin();
}

ActionIterator
ActionWarehouse::inputFileActionsEnd()
{
  return _ordered_actions.end();
}

ActionIterator
ActionWarehouse::allActionsBegin(Parser * p_ptr)
{
  // Clear the current Action vector so that we can fill it up with new ordered actions
  _ordered_actions.clear();

//#ifdef DEBUG
  std::cerr << "Ordered Actions:\n";
//#endif
  
  std::vector<std::set<std::string> > _ordered_names = _actions.getSortedValuesSets();
  for (std::vector<std::set<std::string> >::iterator i = _ordered_names.begin(); i != _ordered_names.end(); ++i)
  {
//#ifdef DEBUG
    std::cerr << "(";
    unsigned int jj = 0;
    for (std::set<std::string>::iterator j = i->begin(); j != i->end(); ++j)
    {
      if (jj++) std::cerr << ", ";
      std::cerr << *j;
    }
    std::cerr << ")\n";
//#endif
    
    for (std::set<std::string>::iterator j = i->begin(); j != i->end(); ++j)
    {
      // Check to see if this action_name is required and there are no blocks
      // to satisfy it
      if (_registered_actions[*j] && _action_blocks[*j].size() == 0)
        // See if the factory has any Actions it can build on the fly to satisfy this action_name
        if (!ActionFactory::instance()->buildAllBuildableActions(*j, p_ptr))
          mooseError(std::string("Unsatisified Action \"") + *j + "\" found during MOOSE problem setup");
      
      
//#ifdef DEBUG
      for (std::vector<Action *>::iterator k = _action_blocks[*j].begin(); k != _action_blocks[*j].end(); ++k)
        std::cerr << "\t" << (*k)->getAction() << "\n";
//#endif
      // append all the Actions that satisfy this "action" onto the end of the ordered_vector
      _ordered_actions.insert(_ordered_actions.end(), _action_blocks[*j].begin(), _action_blocks[*j].end());
    }
  }
  
  
  return _ordered_actions.begin();
}

ActionIterator
ActionWarehouse::allActionsEnd()
{
  return _ordered_actions.end();
}

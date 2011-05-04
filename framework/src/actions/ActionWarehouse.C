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

#include "ActionWarehouse.h"
#include "ActionFactory.h"
#include "Parser.h"

ActionWarehouse::ActionWarehouse() :
  _empty_action(NULL),
  _generator_valid(false),
  _parser_ptr(NULL)
{
}

ActionWarehouse::~ActionWarehouse()
{
}

void
ActionWarehouse::clear()
{
  _action_blocks.clear();
  _generator_valid = false;
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
  std::map<std::string, std::vector<Action *> >::iterator iter;
  
  // We'll use a map to reorder for us
  std::map<std::string, Action *> input_file_blocks;
  std::map<std::string, Action *>::iterator i;
  
  for (iter = _action_blocks.begin(); iter != _action_blocks.end(); ++iter)
    for (std::vector<Action *>::iterator j = iter->second.begin(); j != iter->second.end(); ++j)
      input_file_blocks[(*j)->name()] = *j;

  _ordered_actions.clear();
  for (i = input_file_blocks.begin(); i != input_file_blocks.end(); ++i)
    _ordered_actions.push_back(i->second);

  std::sort(_ordered_actions.begin(), _ordered_actions.end(), InputFileSort());
  
  // We'll push one more "empty" action onto the end so that when we print the input syntax
  // everything will get closed off without any odd tail calls.  Had to do delayed construction
  if (_empty_action == NULL)
  { 
    InputParameters pars = validParams<EmptyAction>();
    _empty_action = ActionFactory::instance()->createNonParsed("finish_input_file_output", pars);       // no memory leak here, this action gets deleted in Actionfactory
  }
  _ordered_actions.push_back(_empty_action);
  
  return _ordered_actions.begin();
}

ActionIterator
ActionWarehouse::inputFileActionsEnd()
{
  return _ordered_actions.end();
}

void
ActionWarehouse::buildBuildableActions(const std::string &action_name)
{
  mooseAssert(_parser_ptr != NULL, "Parser Pointer NULL in ActionWarehouse");
  
  if (_registered_actions[action_name] && _action_blocks[action_name].empty())
    if (!ActionFactory::instance()->buildAllBuildableActions(action_name, _parser_ptr))
      _unsatisfied_dependencies.insert(action_name);
}

void
ActionWarehouse::checkUnsatisfiedActions() const
{
  std::stringstream oss;
  bool empty = true;
  
  for (std::set<std::string>::const_iterator i = _unsatisfied_dependencies.begin(); i != _unsatisfied_dependencies.end(); ++i)
  {
    if (_action_blocks.find(*i) == _action_blocks.end())
    {
      if (empty)
        empty = false;
      else
        oss << " ";
      oss << *i;
    }
  }

  if (!empty)
    mooseError(std::string("The following unsatisfied actions where found while setting up the MOOSE problem:\n")
               + oss.str() + "\n");
}

void
ActionWarehouse::printActionDependencySets()
{
  std::cout << "Ordered Actions:\n";
  
  std::vector<std::set<std::string> > _ordered_names = _actions.getSortedValuesSets();
  for (std::vector<std::set<std::string> >::const_iterator i = _ordered_names.begin(); i != _ordered_names.end(); ++i)
  {
    std::cout << "(";
    unsigned int jj = 0;
    for (std::set<std::string>::const_iterator j = i->begin(); j != i->end(); ++j)
    {
      if (jj++) std::cout << ", ";
      std::cout << *j;
    }
    std::cout << ")\n";
    
    for (std::set<std::string>::const_iterator j = i->begin(); j != i->end(); ++j)
    {
      // Check to see if this action_name is required and there are no blocks
      // to satisfy it
//      if (_registered_actions[*j] && _action_blocks[*j].size() == 0)
        // See if the factory has any Actions it can build on the fly to satisfy this action_name
//        if (!ActionFactory::instance()->buildAllBuildableActions(*j, p_ptr))
//          unsatisfied_dependencies.insert(*j);
      
      for (std::vector<Action *>::const_iterator k = _action_blocks[*j].begin(); k != _action_blocks[*j].end(); ++k)
        std::cout << "\t" << (*k)->getAction() << "\n";

    }
  }
}

ActionWarehouse::iterator
ActionWarehouse::begin()
{
  if (_generator_valid)
    mooseError("Cannont create two active iterators on ActionWarehouse at the same time");
  
  return iterator (*this);
}

ActionWarehouse::iterator
ActionWarehouse::end()
{
  return iterator (*this, true);
}

//--------------------------------------------------------------------------
// Iterator methods
ActionWarehouse::iterator::iterator(ActionWarehouse & act_wh, bool end)
  :_act_wh(act_wh),
   _first(true),
   _end(end)
{
  _act_wh._ordered_names = _act_wh._actions.getSortedValues();

  // current action name
  _i = _act_wh._ordered_names.begin();
  
  _act_wh.buildBuildableActions(*_i);

  _j = _act_wh._action_blocks[*_i].begin();
  
  _act_wh._generator_valid = true;

  // Advanced to the first item
  this->operator++();
}

bool
ActionWarehouse::iterator::operator==(const ActionWarehouse::iterator &rhs) const
{
  return !operator!=(rhs);
}

bool
ActionWarehouse::iterator::operator!=(const ActionWarehouse::iterator &rhs) const
{
  return _end != rhs._end;
}

ActionWarehouse::iterator &
ActionWarehouse::iterator::operator++()
{
  mooseAssert(_act_wh._generator_valid, "Action iterator invalid in ActionWarehouse\n");

  if (_first)
    _first = false;
  else
    ++_j;
  while (_j == _act_wh._action_blocks[*_i].end())
    if (++_i == _act_wh._ordered_names.end())
    {
      _act_wh._generator_valid = false;
      _end = true;
      break;
    }
    else
    {
      _act_wh.buildBuildableActions(*_i);
      _j = _act_wh._action_blocks[*_i].begin();
    }
  
  return *this;
}

Action *
ActionWarehouse::iterator::operator*()
{
  mooseAssert(_act_wh._generator_valid, "Action iterator invalid in ActionWarehouse\n");

  return *_j;
}

//--------------------------------------------------------------------------
// Functor methods

ActionWarehouse::InputFileSort::InputFileSort() 
{ 
  _o.reserve(11);
  _o.push_back("Mesh"); 
  _o.push_back("Variables"); 
  _o.push_back("AuxVariables"); 
  _o.push_back("Kernels"); 
  _o.push_back("AuxKernels"); 
  _o.push_back("BCs"); 
  _o.push_back("AuxBCs"); 
  _o.push_back("Materials"); 
  _o.push_back("Postprocessors"); 
  _o.push_back("Executioner"); 
  _o.push_back("Output"); 
}

bool 
ActionWarehouse::InputFileSort::operator() (Action *a, Action *b) 
{ 
  std::vector<std::string> elements; 
  std::string short_a, short_b; 
  Parser::tokenize(a->name(), elements); 
  short_a = elements[0]; 
  elements.clear(); 
  Parser::tokenize(b->name(), elements); 
  short_b = elements[0]; 
   
  return std::find(_o.begin(), _o.end(), short_a) - std::find(_o.begin(), _o.end(), short_b) >= 0 ? false : true; 
}

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
#include "MooseObjectAction.h"
#include "InputFileFormatter.h"
#include "InputParameters.h"

ActionWarehouse::ActionWarehouse() :
  _generator_valid(false),
  _parser_ptr(NULL),
  _show_actions(false)
{
}

ActionWarehouse::~ActionWarehouse()
{
}

void
ActionWarehouse::clear()
{
  for (std::map<std::string, std::vector<Action *> >::iterator it = _action_blocks.begin(); it != _action_blocks.end(); ++it)
  {
    for (std::vector<Action *>::iterator jt = (*it).second.begin(); jt != (*it).second.end(); ++jt)
      delete *jt;
  }
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
  Parser::tokenize(action_sets, sets, 1, "()");

  for (unsigned int i=0; i<sets.size(); ++i)
  {
    action_names.clear();
    Parser::tokenize(sets[i], action_names, 0, ", ");
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

  // Some error checking
  if (_registered_actions.find(action_name) == _registered_actions.end())
    mooseError("A(n) " << action_name << " is not a registered action name");

  // Make sure that the ObjectAction action_name and Action action_name are consistent
  // otherwise that means that is action was built by the wrong type
  ObjectAction * moa = dynamic_cast<ObjectAction *>(blk);
  if (moa)
  {
    InputParameters mparams = moa->getObjectParams();
    if (mparams.have_parameter<std::string>("built_by_action"))
    {
      std::string moose_action_name = moa->getObjectParams().get<std::string>("built_by_action");
      if (moose_action_name != action_name &&
          moose_action_name != "add_aux_bc" && moose_action_name != "add_aux_kernel") // The exception
        mooseError("Inconsistent Action Name detected! Action that satisfies " + action_name + " is building a MOOSE Object that normally satisfies " + moose_action_name);
    }
  }

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

void
ActionWarehouse::printInputFile(std::ostream & out)
{
  InputFileFormatter tree(out, false);

  std::map<std::string, std::vector<Action *> >::iterator iter;

  std::vector<Action *> ordered_actions;
  ordered_actions.clear();
  for (iter = _action_blocks.begin(); iter != _action_blocks.end(); ++iter)
    for (std::vector<Action *>::iterator j = iter->second.begin(); j != iter->second.end(); ++j)
      ordered_actions.push_back(*j);

  for (std::vector<Action* >::iterator i = ordered_actions.begin();
       i != ordered_actions.end();
       ++i)
  {
    std::string name ((*i)->name());

    if (Moose::syntax.isAssociated(name, false) != "")
    {
      InputParameters params = (*i)->getParams();
      tree.insertNode(name, name, true, &params);
    }
  }

  tree.print();
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
  std::cerr << "[DBG][ACT] Ordered Actions:\n";

  std::vector<std::set<std::string> > _ordered_names = _actions.getSortedValuesSets();
  for (std::vector<std::set<std::string> >::const_iterator i = _ordered_names.begin(); i != _ordered_names.end(); ++i)
  {
    std::cerr << "[DBG][ACT] (";
    unsigned int jj = 0;
    for (std::set<std::string>::const_iterator j = i->begin(); j != i->end(); ++j)
    {
      if (jj++) std::cerr << ", ";
      std::cout << *j;
    }
    std::cout << ")\n";

    for (std::set<std::string>::const_iterator j = i->begin(); j != i->end(); ++j)
    {
      for (std::vector<Action *>::const_iterator k = _action_blocks[*j].begin(); k != _action_blocks[*j].end(); ++k)
        std::cerr << "[DBG][ACT]" << "\t" << (*k)->getAction() << "\n";
    }
  }
}

void
ActionWarehouse::executeAllActions()
{
  if (_show_actions)
  {
    std::cerr << "[DBG][ACT] Action Dependency Sets:\n";
    printActionDependencySets();
    std::cerr << "[DBG][ACT] Executing actions:" << std::endl;
  }

  for (iterator act_iter = begin(); act_iter != end(); ++act_iter)
  {
     // Delay the InputParameters check of MOOSE based objects until just before "acting"
     // so that Meta-Actions can complete the build of parameters as necessary
     ObjectAction * obj_action = dynamic_cast<ObjectAction *>(*act_iter);
     if (obj_action != NULL)
       obj_action->getObjectParams().checkParams(obj_action->name());

     if (_show_actions)
       std::cerr << "[DBG][ACT] - " << (*act_iter)->name() << std::endl;
     // Act!
     (*act_iter)->act();
   }
}

void
ActionWarehouse::executeActionsWithAction(const std::string & name)
{
  for (ActionIterator act_iter = actionBlocksWithActionBegin(name);
       act_iter != actionBlocksWithActionEnd(name);
       ++act_iter)
  {
    // Delay the InputParameters check of MOOSE based objects until just before "acting"
    // so that Meta-Actions can complete the build of parameters as necessary
    ObjectAction * obj_action = dynamic_cast<ObjectAction *>(*act_iter);
    if (obj_action != NULL)
      obj_action->getObjectParams().checkParams(obj_action->name());

    (*act_iter)->act();
  }
}

ActionWarehouse::iterator
ActionWarehouse::begin()
{
  if (_generator_valid)
    mooseError("Cannot create two active iterators on ActionWarehouse at the same time");

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
  if (end) return;

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

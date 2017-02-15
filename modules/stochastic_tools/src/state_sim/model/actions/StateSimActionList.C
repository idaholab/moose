/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "StateSimActionList.h"
#include "StateSimAction.h"
#include <unordered_map>

StateSimActionList::StateSimActionList(bool move_from_current)
  : _move_from_current(move_from_current)
{
}

void
StateSimActionList::add(StateSimAction * to_add)
{
  std::unordered_map<int, StateSimAction *>::const_iterator got = _actions.find(to_add->id());
  if (got == _actions.end())
  {
    _actions.insert(std::make_pair(to_add->id(), to_add));
  }
}

void
StateSimActionList::clear()
{
  _actions.clear();
}

void
StateSimActionList::addRange(StateSimActionList * to_add)
{
  _actions.insert(to_add->_actions.begin(), to_add->_actions.end());
}

bool
StateSimActionList::contains(const StateSimAction to_find)
{
  std::unordered_map<int, StateSimAction *>::const_iterator got = _actions.find(to_find.id());
  return (got != _actions.end());
}

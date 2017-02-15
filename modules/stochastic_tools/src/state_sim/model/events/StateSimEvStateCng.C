/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "StateSimEvStateCng.h"
#include "StateSimState.h"
#include "StateSimBitset.h"
#include "StateSimModel.h"
#include <string>

StateSimEvStateCng::StateSimEvStateCng(StateSimModel & main_model, const std::string &  name)
  : StateSimEvConditionBased(main_model, name, EVENT_TYPE_ENUM::ET_STATE_CNG),
    _if_in_state(true),
    _all_items(true)
{
}

StateSimEvStateCng::StateSimEvStateCng(StateSimModel & main_model, const std::string &  name, std::vector<StateSimState> & key_states, bool if_in_state, bool all_items)
  : StateSimEvConditionBased(main_model, name, EVENT_TYPE_ENUM::ET_STATE_CNG),
    _if_in_state(if_in_state),
    _all_items(all_items)
{
  for (auto state : key_states)
  {
    this->addRelatedItem(state);
  }
}

bool
StateSimEvStateCng::isTriggered(const TimespanH &, const StateSimBitset & cur_states)
{
  bool found = false;
  for (auto item : _related_items)
  {
    int i = item.second->id();
    if ((cur_states.count() > i) && (cur_states[i])) //desired state is in current list
    {
      found = true;

      if (_if_in_state) //We are looking for an item in the list to trigger us
      {
        if (!_all_items) //only one item true needed.
          return true;
      }
      else //don't want the item in order to trigger us but we found it
      {
        if (_all_items) //only one item needed to not trigger.
          return false;
      }
    }
    else //desired state is not in current list
    {
      if (_if_in_state) //We are looking for an item in the list to trigger us
      {
        if (_all_items) //must have all items for a tigger needed.
          return false;
      }
      else //don't want the item in order to trigger and we didn't find it
      {
        if (!_all_items) //only one item needed trigger.
          return true;
      }
    }
  }

  if (_if_in_state)
    return found; //if we didn't kick out early then we went through all the items, as long we found is true then we got a match.
  else
    return !found; //if we didn't kick out early then we went through all the items, as long we found is false then we got a match.
}

void
StateSimEvStateCng::addRelatedItem(StateSimBase & item)
{
  mooseAssert(dynamic_cast<StateSimState*>(&item), "StateSimEvent addRelatedItem() - only states are allowed as related items to a code evaluation.");

  StateSimEvent::addRelatedItem(item);
}

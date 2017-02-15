/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "StateSimState.h"
#include "StateSimEvent.h"
#include "StateSimDiagramEval.h"
#include "StateSimModel.h"
#include <stdexcept>
#include <string>
#include <typeinfo>

StateSimState::StateSimState(StateSimModel & main_model, const std::string & name, STATE_TYPE_ENUM state_type, StateSimDiagram & diagram)
  : StateSimBase(main_model.nextID(STATESIM_TYPE::STATE), name),
    _state_type(state_type),
    _diagram(&diagram)
{
}

void
StateSimState::addEvent(StateSimEvent & event, bool move_from_cur, StateSimAction * act)
{
  mooseAssert(!((!move_from_cur) && (act == NULL)), "StateSimState addEvent() - Need an action for an event that does not move from the state - " + this->name() + " Event - " + this->name());

  _events.push_back(&event);
  auto add_act_list = std::make_shared<StateSimActionList>(move_from_cur);

  //if we have an action add it to the list
  if (act != NULL)
    add_act_list->add(act);

  _ev_actions.push_back(add_act_list);
}

void
StateSimState::addActionForEvent(const StateSimEvent & for_event, StateSimAction & add_action, const bool * move_from_cur)
{
  int idx = -1;

  for (unsigned int i = 0; i < _events.size(); ++i)
  {
    if (_events[i]->id() == for_event.id())
      idx = i;
  }

  mooseAssert(!(idx == -1), "StateSimState AddActionForEvent() - Failed to find the event (" + for_event.name() + ") in the state.");

  if (move_from_cur != NULL)
  {
    _ev_actions[idx]->moveFromCurrent(_ev_actions[idx]->moveFromCurrent() || *move_from_cur);
  }

  auto act_list = _ev_actions[idx];
  act_list->add(&add_action);
}

void
StateSimState::addImmediateAction(StateSimAction & immediate_act)
{
  //TODO
  // if ((_state_type == STATE_TYPE_ENUM::ST_START) && (typeid(immediate_act) == typeid(Sim3DAction))
  // {
  //   throw std::invalid_argument(("Can't have an immediate action with a 3D sim message in a start state");
  // }
  _imediate_actions.add(&immediate_act);
}

const StateSimEvent &
StateSimState::getEvent(unsigned int idx)
{
  mooseAssert((idx < _events.size()), "StateSimState getEvent() - idx is > size of events list.");
  return *_events[idx];
}

StateSimActionList
StateSimState::getEvActionsIdx(unsigned int idx)
{
  mooseAssert((idx < _ev_actions.size()), "StateSimState getEvActionsIdx() - idx is > size of event action list.");
  return *_ev_actions[idx];
}

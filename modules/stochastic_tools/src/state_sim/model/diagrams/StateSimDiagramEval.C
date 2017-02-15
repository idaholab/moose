/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "StateSimDiagramEval.h"
#include "StateSimDiagram.h"
#include "StateSimBitset.h"
#include "StateSimState.h"
#include "StateSimModel.h"
#include <string>
//#include "NextObjID.h"

StateSimDiagramEval::StateSimDiagramEval(StateSimModel & main_model, const std::string & name, DIAGRAM_TYPE_ENUM diag_type)
  : StateSimDiagram(main_model, name, diag_type),
    _diag_type(diag_type)
{
}

State_Val
StateSimDiagramEval::evaluate(const StateSimBitset & cur_states, bool on_success)
{
  if (on_success)
  {
    for (const auto & pair : _ok_states)
    {
      if (cur_states[pair.first])
        return State_Val::TRUE_VAL;
    }

    for (const auto & pair : _fail_states)
    {
      if (cur_states[pair.first])
        return State_Val::FALSE_VAL;
    }

    return State_Val::UNKNOWN_VAL;
  }
  else
  {
    for (const auto & pair : _fail_states)
    {
      if (cur_states[pair.first])
        return State_Val::TRUE_VAL;
    }

    for (const auto & pair : _ok_states)
    {
      if (cur_states[pair.first])
        return State_Val::FALSE_VAL;
    }

    return State_Val::UNKNOWN_VAL;
  }
}

bool
StateSimDiagramEval::isFailedState(int state_id)
{
  const auto & got = _fail_states.find(state_id);
  return got != _fail_states.end();
}

void
StateSimDiagramEval::addState(StateSimState & add_state, const State_Val & state_value)
{
  //add or replace the item with that id
  _states[add_state.id()] = &add_state;
  if (state_value == State_Val::TRUE_VAL)
  {
    //add or replace the item in ok states
    _ok_states[add_state.id()] = &add_state;
  }
  else if (state_value == State_Val::FALSE_VAL)
  {
    //add or replace the item in the fail states
    _fail_states[add_state.id()] = &add_state;
  }
}

std::vector<int>
StateSimDiagramEval::stateIDs()
{
  std::vector<int> ret_list;
  ret_list.reserve(_states.size()); //optimze for the size
  for (const auto & pair : _states)
    ret_list.push_back(pair.first);

  return ret_list;
}

std::vector<StateSimState *>
StateSimDiagramEval::states()
{
  std::vector<StateSimState *> ret_list;
  ret_list.reserve(_states.size()); //optimze for the size
  for (const auto pair : _states)
    ret_list.push_back(pair.second);

  return ret_list;
}

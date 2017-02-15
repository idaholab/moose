/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "StateSimEvLogicEval.h"
#include "StateSimBase.h"
#include "StateSimState.h"
#include "StateSimModel.h"
#include <string>

StateSimEvLogicEval::StateSimEvLogicEval(StateSimModel & main_model, const std::string & name)
  : StateSimEvConditionBased(main_model, name, EVENT_TYPE_ENUM::ET_LOGIC_EVAL),
    _unassigned_top(main_model, "unassigned"),
    _success_space(true),
    _logic_top(_unassigned_top)
{
}

StateSimEvLogicEval::StateSimEvLogicEval(StateSimModel & main_model, const std::string & name, bool success_space, const StateSimLogicNode & logic_top)
  : StateSimEvConditionBased(main_model, name, EVENT_TYPE_ENUM::ET_LOGIC_EVAL),
    _unassigned_top(main_model, "unassigned"),
    _success_space(success_space),
    _logic_top(logic_top)
{
  autoAddRelatedComponents(logic_top);
}

bool
StateSimEvLogicEval::isTriggered(const TimespanH &, const StateSimBitset & cur_states)
{
  if (&_logic_top == &_unassigned_top)
    mooseAssert(false, "StateSimEvLogicEval isTriggered() - Unassigned logic_top.");
  bool eval_res = _logic_top.evaluate(cur_states, _success_space);
  return eval_res; //two lines to help debuging
}

void
StateSimEvLogicEval::autoAddRelatedComponents(const StateSimLogicNode & logic_top)
{
  for (auto state : logic_top.allUsedStates())
    addRelatedItem(*state);
}

void
StateSimEvLogicEval::addRelatedItem(StateSimBase & item)
{
  mooseAssert(dynamic_cast<StateSimDiagramEval *>(&item), "StateSimEvent addRelatedItem() - only evalutation diagrams are allowed as related items to a code evaluation.");

  StateSimEvent::addRelatedItem(item);
}

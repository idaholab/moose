/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "StateSimEvCodeEval.h"
#include "StateSimModel.h"
#include <string>

StateSimEvCodeEval::StateSimEvCodeEval(StateSimModel & main_model, const std::string & name)
  : StateSimEvConditionBased(main_model, name, EVENT_TYPE_ENUM::ET_CODE_EVAL),
    _code_eval(NULL)
{
}

StateSimEvCodeEval::StateSimEvCodeEval(StateSimModel & main_model, const std::string & name, CodeEvalFunc code_eval, std::vector<StateSimVariable> * watch_list)
  : StateSimEvConditionBased(main_model, name, EVENT_TYPE_ENUM::ET_CODE_EVAL),
    _code_eval(code_eval)
{
  if (watch_list)
  {
    for (auto var : *watch_list)
    {
      this->addRelatedItem(var);
    }
  }
}

bool
StateSimEvCodeEval::isTriggered(const TimespanH & cur_ev_time, const StateSimBitset & cur_states)
{
  //if (_code_eval == NULL) throw a problem
  mooseAssert(_code_eval, "StateSimEvCodeEval isTriggered() - code_eval not assigned"); //done in if for debuging

  bool eval_res = _code_eval(cur_ev_time, 0, cur_states, _main_model._variables); //todo get next event time
  return eval_res;                                                                //two lines to help debuging
}

void
StateSimEvCodeEval::addRelatedItem(StateSimBase & item)
{
  mooseAssert(dynamic_cast<StateSimVariable *>(&item), "StateSimEvent addRelatedItem() - only variables are allowed as related items to a code evaluation.");

  StateSimEvent::addRelatedItem(item);
}

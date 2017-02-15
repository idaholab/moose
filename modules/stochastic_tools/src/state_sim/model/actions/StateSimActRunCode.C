/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "StateSimActRunCode.h"
#include "StateSimModel.h"
#include <string>
#include <typeinfo>
#include <stdexcept>
#include <algorithm>

StateSimActRunCode::StateSimActRunCode(StateSimModel & main_model, const std::string &  name, ActRunCodeFunc run_code)
  : StateSimAction(main_model, name, ACTION_TYPE_ENUM::AT_RUN_CODE),
    _run_code(run_code)
{
}

void
StateSimActRunCode::runCode(const TimespanH & cur_time, const TimespanH & next_ev_time, const StateSimBitset & cur_states, StateSimModel & main_model)
{
  _run_code(cur_time, next_ev_time, cur_states, main_model._variables);
}

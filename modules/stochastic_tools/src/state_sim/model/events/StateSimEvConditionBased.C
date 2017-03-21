/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "StateSimEvConditionBased.h"
#include "StateSimModel.h"
#include <string>

StateSimEvConditionBased::StateSimEvConditionBased(StateSimModel & main_model, const std::string &  name, EVENT_TYPE_ENUM ev_type)
  : StateSimEvent(main_model, name, ev_type),
    _occured(false),
    _main_model(main_model)
{
}

/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "StateSimEvTimer.h"
#include "StateSimModel.h"

StateSimEvTimer::StateSimEvTimer(StateSimModel & main_model, const std::string &  name, TimespanH time)
  : StateSimEvTimeBased(main_model, name, EVENT_TYPE_ENUM::ET_STATE_CNG),
    _time(time)
{
}

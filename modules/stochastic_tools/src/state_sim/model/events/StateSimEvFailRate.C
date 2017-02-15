/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "StateSimEvFailRate.h"
#include "StateSimBitset.h"
#include "StateSimModel.h"
#include <string>
#include <limits.h>
#include "MooseRandom.h"

StateSimEvFailRate::StateSimEvFailRate(StateSimModel & main_model, const std::string & name)
  : StateSimEvTimeBased(main_model, name, EVENT_TYPE_ENUM::ET_FAIL_RATE),
    _lambda(0.0),
    _time_rate(0.0),
    _comp_mission_time(0.0)
{
  //todo mark as not loaded;
}

StateSimEvFailRate::StateSimEvFailRate(StateSimModel & main_model, const std::string & name, const Real & lambda_or_freq, const TimespanH & lambda_time_rate, const TimespanH & comp_mission_time)
  : StateSimEvTimeBased(main_model, name, EVENT_TYPE_ENUM::ET_FAIL_RATE),
    _lambda(lambda_or_freq),
    _time_rate(lambda_time_rate),
    _comp_mission_time(comp_mission_time)
{
}

TimespanH
StateSimEvFailRate::nextTime()
{
  Real rand_num = MooseRandom::rand();

  Real temp_d = std::log(1 - rand_num) * -1;
  TimespanH time_to_fail = _time_rate;
  time_to_fail *= (temp_d / _lambda);

  if (time_to_fail > _comp_mission_time)
    return _comp_mission_time + StateSimBase::YEAR_TIME; //return longer than the mission time.
  else
    return time_to_fail;
}

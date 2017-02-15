/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "StateProcessor.h"
#include <string>
#include <cmath>
#include "MooseRandom.h"
#include "StateSimEvFailRate.h"
#include "InputParameterWarehouse.h"
#include "StateSimEvConditionBased.h"
#include <typeinfo>

StateProcessor::StateProcessor(const int & seed, StateSimModel & state_sim_model)
  : _max_sim_time(state_sim_model._max_sim_time),
    _state_sim_model(state_sim_model),
    _cur_ev_time(0),
    _cur_states(100)

{
  MooseRandom::seed(seed);

  for (const auto & pair : _state_sim_model._time_events)
  {
    StateSimEvent & t_ev = *(pair.second);
    TimespanH addI = ((StateSimEvTimeBased &)t_ev).nextTime();
    if (addI <= _max_sim_time)
      addEv(addI);
  }
}

//Add the event in to the list in the proper order
void
StateProcessor::addEv(TimespanH ev_time)
{
  unsigned long idx = 0;
  while ((idx < _ev_times.size()) && (_ev_times[idx] > ev_time))
    ++idx;

  _ev_times.insert(_ev_times.begin() + idx, ev_time);
}

TimespanH
StateProcessor::nextTime()
{
  if (_ev_times.size() > 0)
    return _ev_times.back();
  else
    return StateSimBase::ZERO_TIME;
}

TimespanH
StateProcessor::processNext(TimespanH proc_time)
{
  //remove old items from the list
  if (proc_time >= _ev_times.back())
  {
    _cur_ev_time = _ev_times.back();
    _ev_times.pop_back();
  }

  //todo run state processing

  //return the next item.
  if (_ev_times.size() > 0)
    return _ev_times.back();
  else
    return StateSimBase::ZERO_TIME;
}

bool
StateProcessor::hadCondEv()
{
  for (const auto & pair : _state_sim_model._condition_events)
  {
    StateSimEvent & t_ev = *(pair.second);
    mooseAssert(dynamic_cast<StateSimEvConditionBased *>(&t_ev) != NULL, "Non Condition Event in the condition event list");

    if (((StateSimEvConditionBased &)t_ev).isTriggered(_cur_ev_time, _cur_states))
      return true;
  }

  return false;
}

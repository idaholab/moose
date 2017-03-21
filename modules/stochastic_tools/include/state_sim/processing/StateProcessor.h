/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef STATEPROCESSOR_H
#define STATEPROCESSOR_H

#include <string>
#include <vector>
// #include "StateSimEvTimeBased.h"
// #include "StateSimEvConditionBased.h"
#include "StateSimModel.h"
#include "StateSimBase.h"
#include "StateSimBitset.h"


class StateSimEvTimeBased;
class StateSimEvConditionBased;
/**
 Object to load and run and manage a state simulation model.  TODO - currently this is just for testing the StateSim model objects
 */
class StateProcessor
{
public:
  /**
   * Default constructor all derived objects use
   * @param seed - initial seed to use
   * @param sate_sim_model - the model to be run
   */
  StateProcessor(const int & seed, StateSimModel & state_sim_model);

  /**
   * nextTime - Get the time of the next event.
   * @return the time of the next event.
   */
  TimespanH nextTime();

  /**
   * processNext - process and remove then next event if it's time is < the proc_time.
   * @param proc_time - Max time of next process event.
   * @return - return the time of the next event
   */
  TimespanH processNext(TimespanH proc_time);

  /**
   * hadCondEv - check to see if any conditional events occured.
   * @return - if a conditional event occured
   */
  bool hadCondEv();

private:
  const TimespanH _max_sim_time;
  std::vector<TimespanH> _ev_times;
  StateSimModel & _state_sim_model;
  TimespanH _cur_ev_time;
  StateSimBitset _cur_states;

  /**
   * addEv - Add the event in to the list in the proper order
   * @param ev_time - the time for this event.
   */
  void addEv(TimespanH ev_time);

};

#endif

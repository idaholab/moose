/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef STATESIMACTRUNCODE_H
#define STATESIMACTRUNCODE_H

#include "StateSimAction.h"
#include <string>
#include <vector>

//Forward Declarations
class StateSimState;
class StateSimVariable;
class StateSimBase;
class StateSimBitset;
class StateSimAllVariables;
class StateSimModel;

/**
  * Funcion header for user to impliment, this action runs it when the model specifies.
  * @param cur_ev_time - time of the last event executed, not the current timestep of moose.
  * @param next_ev_time - the time of the next StateSim event.
  * @param cur_states - ID bitset of the current states
  * @param vars - the variables for the model
  */
using ActRunCodeFunc = void (*)(const TimespanH & cur_ev_time, const TimespanH & next_ev_time, const StateSimBitset & cur_states, StateSimAllVariables & vars);

class StateSimActRunCode : public StateSimAction
{
public:
  /**
   * This is the main constructor for the user.
   * @param main_model - the model this action belongs to
   * @param name - name of the item
   * @param run_code - user defined function to execute for this action.
   */
  StateSimActRunCode(StateSimModel & main_model, const std::string & name, ActRunCodeFunc run_code);
  ACTION_TYPE_ENUM getActType()
  {
    return _act_type;
  }

  //todo? turn this into a friend funcion for only the StateProcessor to call
  /**
   * runCode - execute the code or function assigned to this action.
   * @param cur_time - the current time of the simulation
   * @param next_ev_time - the time of the next event in the timeline
   * @param cur_states - the current state list of the simulation
   * @param main_model - the model running for the simulation
   */
  virtual void runCode(const TimespanH & cur_time, const TimespanH & next_ev_time, const StateSimBitset & cur_states, StateSimModel & main_model);

  //TODO
  //getDerivedJSON
  //getJSON
  //deserializeDerived
  //loadObjectLinks

protected:
  ActRunCodeFunc _run_code;
};

#endif

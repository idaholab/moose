/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef STATESIMEVCONDITIONBASED_H
#define STATESIMEVCONDITIONBASED_H

#include "StateSimEvent.h"
#include <string>

//Forward Declarations
class StateSimAllVariables;
class StateSimBitset;
class StateSimModel;

/**
 * Virtual Root class for all condition based Events. (CodeEval, LogicEval, StateCng)
 */
class StateSimModel;
class StateSimEvConditionBased : public StateSimEvent
{
public:
  /**
   * This is the main construter for the user.
   * @param name - name of the item
   * @param ev_type - type of condition event.
   */
  StateSimEvConditionBased(StateSimModel & main_model, const std::string &  name, EVENT_TYPE_ENUM ev_type);

  /**
   * isTriggered - is this event triggered.
   * @param cur_ev_time - time of the last event that occured
   * @param cur_states - states in the current state list
   * @param main_model - the model being simulated
   */
  virtual bool isTriggered(const TimespanH & cur_ev_time, const StateSimBitset & cur_states) = 0;

  /**
   * addRelatedItem - Add related items that can cause this event to occure if they change.
   * @param item - item that is related
   */
  virtual void addRelatedItem(StateSimBase & item) = 0;

protected:
  bool _occured;
  StateSimModel & _main_model;
};

#endif

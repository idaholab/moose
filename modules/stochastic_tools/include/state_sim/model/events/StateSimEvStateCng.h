/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef STATESIMEVSTATECNG_H
#define STATESIMEVSTATECNG_H

#include "StateSimEvConditionBased.h"
#include <string>
#include <vector>

//Forward Declarations
class StateSimState;
class StateSimModel;

/**
 * A State Change event is a condition event that is triggered when either entering or exiting user defined state/s.
 */
class StateSimEvStateCng : public StateSimEvConditionBased
{
public:
  /**
   * This constructor should only to be used by the auto model generation from the JSON file.
   * used to create a temporary holder with the data to be filled later.
  */
  StateSimEvStateCng(StateSimModel & main_model, const std::string &  name);

  /**
   * This is the main construter for the user.
   * @param name - name of the item
   * @param key_states - key states to evaluate against the current state list
   * @param if_in_state - (true) triggered if given state\s are added to the current states. (false) triggered when given state\s are no longer in the current states.
   * @param _all_items - (true) all states are required to trigger the event. (false) only one state is required to trigger the event
   */
  StateSimEvStateCng(StateSimModel & main_model, const std::string &  name, std::vector<StateSimState> & key_states, bool if_in_state = true, bool _all_items = true);

  /**
   * addRelatedItem - Add related items that can cause this event to occure if they change.
   * @param item - item that is related
   */
  void addRelatedItem(StateSimBase & item) override;

  /**
   * isTriggered - is this event triggered.
   * @param cur_ev_time - time of the last event that occured
   * @param cur_states - states in the current state list
   * @param main_model - the model being simulated
   */
  virtual bool isTriggered(const TimespanH & cur_ev_time, const StateSimBitset & cur_states) override;

  //TODO
  //getDerivedJSON
  //deserializeDerived
  //loadObjLinks
  //lookupRelatedItems

  protected:
  bool _if_in_state;
  bool _all_items;

};

#endif

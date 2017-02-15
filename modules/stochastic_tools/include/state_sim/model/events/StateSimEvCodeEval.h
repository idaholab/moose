/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef STATESIMEVCODEEVAL_H
#define STATESIMEVCODEEVAL_H

#include "StateSimEvConditionBased.h"
#include <string>

using CodeEvalFunc = bool (*)(const TimespanH & cur_ev_time, const TimespanH & next_ev_time, const StateSimBitset & cur_states, const StateSimAllVariables & vars);

//Forward Declarations
class StateSimAllVariables;
class StateSimBitset;
class StateSimVariable;
class StateSimModel;

/**
 * Code Evaluation Event, to be used for evaluating a user defined code to trigger state assigned actions.
 */
class StateSimEvCodeEval : public StateSimEvConditionBased
{
public:
  /**
   * This constructor should only to be used by the auto model generation from the JSON file.
   * used to create a temporary holder with the data to be filled later.
  */
  StateSimEvCodeEval(StateSimModel & main_model, const std::string & name);

  /**
   * This is the main constructor
   * @param name is thename of the
  */
  StateSimEvCodeEval(StateSimModel & main_model, const std::string & name, CodeEvalFunc code_eval, std::vector<StateSimVariable> * watch_list = NULL); //watchlist

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
  CodeEvalFunc _code_eval;
};

#endif

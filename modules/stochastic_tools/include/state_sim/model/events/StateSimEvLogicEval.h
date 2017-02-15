/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef STATESIMEVLOGICEVAL_H
#define STATESIMEVLOGICEVAL_H

#include "StateSimEvConditionBased.h"
#include "StateSimLogicNode.h" //cant use Forward decleration
#include <string>

//Forward Declarations
class StateSimModel;

/**
 * A Logic Evaluation event is a condition event that is triggered when the assosiated logic tree top evaluates to TRUE.
 */
class StateSimEvLogicEval : public StateSimEvConditionBased
{
public:
  /**
   * This constructor should only to be used by the auto model generation from the JSON file.
   * used to create a temporary holder with the data to be filled later.
  */
  StateSimEvLogicEval(StateSimModel & main_model, const std::string & name);

  /**
   * This is the main construter to be used by the user.
   * @param name - name of the item
   * @param success_space - triggered if logic evaluates resulted in true
   * @param logic_top - Top logic gate to evaluate.
   */
  StateSimEvLogicEval(StateSimModel & main_model, const std::string & name, bool success_space, const StateSimLogicNode & logic_top);

  /**
   * isTriggered - Evaluate if this event is triggered.
   * @param cur_ev_time - time of the most recent event that occured
   * @param cur_states - states this model is currently in.
   * @param main_model - model that is being simulated.
   */
  virtual bool isTriggered(const TimespanH & cur_ev_time, const StateSimBitset & cur_states) override;

  /**
   * addRelatedItem - Add related items that can cause this event to occure if they change.
   * @param item - item that is related
   */
  void addRelatedItem(StateSimBase & item) override;

  //TODO
  //getDerivedJSON
  //deserializeDerived
  //loadObjLinks
  //lookupRelatedItems

  //void setSuccessSpace(bool success_space) { _success_space = success_space; };
  //void setLogicTop();

protected:
  StateSimLogicNode _unassigned_top;
  bool _success_space;
  const StateSimLogicNode & _logic_top;

  /**
   * autoAddRelatedComponents - Add all the components in the given logic tree to this event.
   * @param logic_top - logic tree top to traverse for components to add
   */
  void autoAddRelatedComponents(const StateSimLogicNode & logic_top);
};

#endif

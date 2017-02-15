/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef STATESIMACTTRANSITION_H
#define STATESIMACTTRANSITION_H

#include "StateSimAction.h"
#include <string>
#include <vector>

//Forward Declarations
class StateSimState;
class StateSimVariable;

class StateSimActTransition : public StateSimAction
{
public:
  /**
   * This is the basic construter but transition data must be added with function later.
   * @param main_model - the model this action belongs to
   * @param name - name of the item
   */
  StateSimActTransition(StateSimModel & main_model, const std::string & name);

  /**
   * This is the main construter for the user.
   * @param main_model - the model this action belongs to
   * @param name - name of the item
   * @param to_state - state to move to for this transition.
   * @param move_desc - description of action that caused this transition.
   */
  StateSimActTransition(StateSimModel & main_model, const std::string & name, StateSimState & to_state, const std::string & move_desc = "");
  ACTION_TYPE_ENUM getActType()
  {
    return _act_type;
  }

  bool getMutuallyExclusive()
  {
    return _mut_excl;
  }
  void setMutuallyExclusive(bool value)
  {
    _mut_excl = value;
  }

  /**
   * addToState - add a destination state for this transition with a dynamic probabilty of movement.
   * @param to_state - state to move to
   * @param state_sim_var_prob - dyamic probability of moving to the state if more than one destination state
   * @param move_desc - description of why this movement occured
   */
  void addToState(StateSimState & to_state, StateSimBase * state_sim_var_prob, const std::string & move_desc = "");

  /**
   * addToState - add a destination state for this transition with a static probabilty.
   * @param to_state - state to move to
   * @param prob - probability of moving to the state if more than one destination state
   * @param move_desc - description of why this movement occured
   */
  void addToState(StateSimState & to_state, const Real & prob = -1, const std::string & move_desc = "");

  /**
   * removeToState - remove the given state from the destination list.
   * @param rem_state - state to remove to
   */
  void removeToState(StateSimState & rem_state);

  /**
   * WhichToState - return (sample if needed) on which state/s to go to from this state
   * @return vector of StateSimIdAndDesc on which state/s to go to.
   */
  std::vector<std::shared_ptr<StateSimIdAndDesc>> WhichToState();

  //TODO
  //getDerivedJSON
  //getJSON
  //deserializeDerived
  //loadObjectLinks

protected:
  std::vector<StateSimState *> _to_states;             //possible states to travel to for this transition
  std::vector<Real> _to_state_probs;                   //probability of moving to each of the states in _new_state_ids
  std::vector<StateSimVariable *> _to_state_var_probs; //dynamic probability of moving to each of the states
  std::vector<std::string> _fail_desc;                 //description of the failure for traceing

  bool _mut_excl;
  bool _has_var_probs;
};

#endif

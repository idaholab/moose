/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef STATESIMDIAGRAMEVAL_H
#define STATESIMDIAGRAMEVAL_H

#include "StateSimDiagram.h"
#include <unordered_map>
#include <string>
#include <vector>

//Forward Declarations
class StateSimState;
class StateSimBitset;

enum class State_Val
{
  TRUE_VAL,
  FALSE_VAL,
  UNKNOWN_VAL
};

/**
 * Eval diagrams are model diagrams that can be evaluated, returning a boolean value according to which
 * of their states are in the current state list. (Component and System Diagrams)
 */
class StateSimDiagramEval : public StateSimDiagram
{
public:
  /**
   * This is the main construter for the user.
   * @param name - name of the item
   * @param diag_type - type of evaluation diaram.
   */
  StateSimDiagramEval(StateSimModel & main_model, const std::string & name, DIAGRAM_TYPE_ENUM diag_type);
  virtual DIAGRAM_TYPE_ENUM getEvType() {return _diag_type;}

  /**
   * evaluate - returns if the state is in the failed state list.
   * @cur_states - list of the current states
   * @return  the enum State_Val of the state
   */
  State_Val evaluate(const StateSimBitset & cur_states, bool on_success);

  /**
   * isFailedState - returns if the state is in the failed state list.
   * @return  if the state is in the failed state list.
   */
  bool isFailedState(int state_id);

  /**
   * addState - add a state to the diagram.
   * @param add_state - state to add
   * @param state_value - the value associated with the state
   */
  void addState(StateSimState & add_state, const State_Val & state_value);

  /**
   * stateIDS - Get the ids of the states in the diagram
   * @return vector of state IDs
   */
  std::vector<int> stateIDs();

  /**
   * stateIDS - Get the states in the diagram
   * @return vector of the states
   */
  std::vector<StateSimState *> states();

  //todo
  //getDerivedJSON
  //deserializeDerived
  //loadObjLinks

protected:
  DIAGRAM_TYPE_ENUM _diag_type;
  std::unordered_map<int, StateSimState *> _ok_states;
  //std::unordered_map<int, StateSimState *> _single_state_group;
  std::unordered_map<int, StateSimState *> _fail_states;
};

#endif

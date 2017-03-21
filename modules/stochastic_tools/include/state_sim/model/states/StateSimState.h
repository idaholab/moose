/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef STATESIMSTATE_H
#define STATESIMSTATE_H

#include "StateSimBase.h"
#include "StateSimActionList.h"
#include <string>

//Forward Declarations
class StateSimDiagram;
class StateSimDiagramEval;
class StateSimEvent;
class StateSimModel;

/**
 * A StateSim model is make up of States with Immediate Actions and Event driven Actions.  Related states are
 *  clustered into different types of diagrams.
 */
class StateSimState : public StateSimBase
{
public:
  /**
   * This is the main construter for the user.
   * @param name - name of the item
   * @param state_type - type of state.
   * @param diagram - owner diagram.
   */
  StateSimState(StateSimModel & main_model, const std::string & name, STATE_TYPE_ENUM state_type, StateSimDiagram & diagram);

  /**
   * This function returns the state type
   * @return - the type of state STATE_TYPE_ENUM(ST_START, ST_STANDARD, ST_KEYSTATE, ST_TERMINAL)
   */
  STATE_TYPE_ENUM getStateType() {return _state_type;}

  /**
   * This function returns the number of events assigned to this state
   * @return - the number of events
   */
  int eventCount(){return _events.size();}

  /**
   * This funcion adds an event and an optional linked action
   * @param event - event to add
   * @param move_from_cur - leave this state if the event occurs
   * @param act - action to execute if the event occurs
   */
  void addEvent(StateSimEvent & event, bool move_from_cur = false, StateSimAction * act = NULL);

  /**
   * This funcion adds an action to an existing event, an event can have more than one action
   * @param for_event - which event
   * @param add_action - action to add to the event
   * @param move_from_cur - - leave this state if the event occurs
   */
  void addActionForEvent(const StateSimEvent & for_event, StateSimAction & add_action, const bool * move_from_cur = NULL);

  /**
   * This funcion adds an immediate action for this state this action is executed as soon as this state is entered.
   * @param immediate_act - action to execute upon entering this state
   */
  void addImmediateAction(StateSimAction & immediate_act);

  /**
   * This funcion returns a copy of the list of immediate actions, the list references the actual actions
   * @return a StateSimActionList referencing the actions to be taken when the state is entered
   */
  StateSimActionList getimmediateActions() {return _imediate_actions;};

  /**
   * This funcion returns an event for the given index, causes an assert if index is not valid.
   * @return a const StateSimEvent referencing the event for he given index
   */
  const StateSimEvent & getEvent(unsigned int idx);

  /**
   * This funcion returns a list of actions for the given event index.
   * @return a StateStateSimActionList referencing the actions for the given event index.
   */
  StateSimActionList getEvActionsIdx(unsigned int idx);


  //TODO
  //getDerivedJSON
  //getJSON
  //deserializeDerived
  //for gui lookupRelatedItems
protected:
  //std::string geometry; //used for display
  STATE_TYPE_ENUM _state_type;
  StateSimDiagram * _diagram;
  StateSimActionList _imediate_actions;
  std::vector<StateSimEvent *> _events;
  std::vector<std::shared_ptr<StateSimActionList>> _ev_actions;
};

#endif

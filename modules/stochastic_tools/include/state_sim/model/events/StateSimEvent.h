/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef STATESIMEVENT_H
#define STATESIMEVENT_H

#include "StateSimBase.h"
#include <string>
#include <unordered_map>

//Forward Declarations
class StateSimModel;

/**
 * Virtual Root class for all Events.
 */
class StateSimEvent : public StateSimBase
{
public:
  /**
   * This is the main construter by he derived classes.
   * @param name - name of the item
   * @param ev_type - type of event given by the derived class.
   */
  StateSimEvent(StateSimModel & main_model, const std::string & name, EVENT_TYPE_ENUM ev_type);
  virtual ~StateSimEvent() = 0; //to make abstract

  //todo
  //void AddRelatedItems(int item_id);
  //getDerivedJSON
  //getJSON
  //for gui lookupRelatedItems

protected:
  EVENT_TYPE_ENUM _ev_type;
  std::unordered_map<int, StateSimBase *> _related_items;

  /**
   * addRelatedItem - Add related items that can cause this event to occure if they change.
   * @param item - item that is related
   */
  virtual void addRelatedItem(StateSimBase & item);
};

#endif

/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef STATESIMEVTIMEBASED_H
#define STATESIMEVTIMEBASED_H

#include "StateSimEvent.h"

//Forward Declarations
class StateSimModel;

/**
 * Root class for all time based Events. (FailRate, Timer)
 */
class StateSimEvTimeBased : public StateSimEvent
{
public:
  /**
   * This is the main construter for the user.
   * @param name - name of the item
   * @param ev_type - type of event given by the derived class.
   */
  StateSimEvTimeBased(StateSimModel & main_model, const std::string &  name, EVENT_TYPE_ENUM ev_type);

  /**
   * nextTime - The time this event is going to occure.
   * @return The time this event is going to occure.
   */
  virtual TimespanH nextTime() = 0;
};

#endif

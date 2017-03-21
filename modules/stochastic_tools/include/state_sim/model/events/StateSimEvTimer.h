/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef STATESIMEVTIMER_H
#define STATESIMEVTIMER_H

#include "StateSimEvTimeBased.h"

//Forward Declarations
class StateSimModel;

/**
 * A timer event is a time based event that is triggered when the defined amount of time is passed since it's owner state was
 *  added to the current state list.
 */
class StateSimEvTimer : public StateSimEvTimeBased
{
public:
  /**
   * This is the main construter for the user.
   * @param name - name of the item
   * @param time - time from when parent state is entered for the event to occure.
   */
  StateSimEvTimer(StateSimModel & main_model, const std::string &  name, TimespanH time = 0.0);

  /**
   * nextTime - The time this event is going to occure.
   * @return The time this event is going to occure.
   */
  virtual TimespanH nextTime() override {return _time;};



  //todo
  //getDerivedJSON
  //deserializeDerived

protected:
  TimespanH _time;
};

#endif

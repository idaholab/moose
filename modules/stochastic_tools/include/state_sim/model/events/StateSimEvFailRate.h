/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef STATESIMEVFAILRATE_H
#define STATESIMEVFAILRATE_H

#include "StateSimEvTimeBased.h"

//Forward Declarations
class StateSimModel;

/**
 * A Failure Rate event is a time based that samples the next time the event occures over a normal distribution using the given perameters
 */
class StateSimEvFailRate: public StateSimEvTimeBased
{
public:
  /**
   * This constructor should only to be used by the auto model generation from the JSON file.
   * used to create a temporary holder with the data to be filled later.
  */
  StateSimEvFailRate(StateSimModel & main_model, const std::string & name);

  /**
   * This is the main construter for the user.
   * @param name - name of the item
   * @param lambda_or_freq - failure rate lambda value
   * @param lambda_time_rate - time rate for the lambda value (typically every 24 hours or 365 days)
   * @param comp_mission_time - component mission time (typically  24 hours)
   */
  StateSimEvFailRate(StateSimModel & main_model, const std::string & name, const Real & lambda_or_freq, const TimespanH & lambda_time_rate, const TimespanH & comp_mission_time);

  /**
   * nextTime - The time this event is going to occure.
   * @return The time this event is going to occure.
   */
  virtual TimespanH nextTime() override;

protected:
  Real _lambda;
  TimespanH _time_rate;
  TimespanH _comp_mission_time;
};

#endif

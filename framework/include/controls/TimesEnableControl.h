//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ConditionalEnableControl.h"

class Times;

/**
 * Control for enabling/disabling objects when near or past times from a Times object
 */
class TimesEnableControl : public ConditionalEnableControl
{
public:
  static InputParameters validParams();

  TimesEnableControl(const InputParameters & parameters);

protected:
  virtual bool conditionMet(const unsigned int & i) override;

private:
  /// The time object providing the times
  const Times & _times;

  /// The tolerance on hitting time points with the current simulation time
  const Real _time_window;

  /// Whether to consider that going past a time point should trigger the control
  const bool _act_on_time_stepping_across_time_point;

  /// To keep track of the current threshold to hit
  Real _prev_time_point_current;
  /// To keep track of the next threshold to hit
  Real _prev_time_point;
  /// To keep track of the current time step
  Real _t_current;
};

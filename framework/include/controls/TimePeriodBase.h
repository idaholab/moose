//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "ConditionalEnableControl.h"

/**
 * Base class for basic control for disabling objects for a portion of the simulation.
 */
class TimePeriodBase : public ConditionalEnableControl
{
public:
  /**
   * Class constructor
   * @param parameters Input parameters for this Control object
   */
  static InputParameters validParams();

  TimePeriodBase(const InputParameters & parameters);

protected:
  /**
   * Helper base method to set start and end times for controls.
   */
  void setupTimes();

  virtual bool conditionMet(const unsigned int & i) = 0;

  /// The time to begin enabling the supplied object tags (defaults to the simulation start time)
  std::vector<Real> _start_time;

  /// The time to stop enabling the supplied object tags (defaults to the end of the simulation)
  std::vector<Real> _end_time;
};

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
#include "TimePeriodBase.h"

/**
 * A basic control for disabling objects for a portion of the simulation.
 */
class TimePeriod : public TimePeriodBase
{
public:
  /**
   * Class constructor
   * @param parameters Input parameters for this Control object
   */
  static InputParameters validParams();

  TimePeriod(const InputParameters & parameters);

protected:
  /**
   * If enabled, this injects the start/end times into the TimeStepper sync times.
   */
  void initialSetup() override;

  virtual bool conditionMet(const unsigned int & i) override;
};

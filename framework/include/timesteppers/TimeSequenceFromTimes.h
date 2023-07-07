//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TimeSequenceStepperBase.h"

class Times;

/**
 * Time sequence stepper that gets its sequence of times to hit from a times object
 */
class TimeSequenceFromTimes : public TimeSequenceStepperBase
{
public:
  static InputParameters validParams();

  TimeSequenceFromTimes(const InputParameters & parameters);

protected:
  void step() override;

  /// A Times object that will provide the sequence of times to hit
  const Times & _times;
};

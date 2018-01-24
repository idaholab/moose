//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef TIMESEQUENCESTEPPER_H
#define TIMESEQUENCESTEPPER_H

#include "TimeSequenceStepperBase.h"

class TimeSequenceStepper;

template <>
InputParameters validParams<TimeSequenceStepper>();

/**
 * Solves the PDEs at a sequence of time points given as a vector in the input file.
 * Adjusts the time sequence vector according to Transient start_time and end_time.
 */
class TimeSequenceStepper : public TimeSequenceStepperBase
{
public:
  TimeSequenceStepper(const InputParameters & parameters);

  virtual void init() override;
};

#endif // TIMESEQUENCESTEPPER_H

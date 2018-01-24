/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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

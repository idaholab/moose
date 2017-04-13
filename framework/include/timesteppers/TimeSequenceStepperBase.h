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

#ifndef TIMESEQUENCESTEPPERBASE_H
#define TIMESEQUENCESTEPPERBASE_H

#include "TimeStepper.h"

class TimeSequenceStepperBase;

template <>
InputParameters validParams<TimeSequenceStepperBase>();

/**
 * Solves the PDEs at a sequence of given time points.
 * Adjusts the time sequence vector according to Transient start_time and end_time.
 */
class TimeSequenceStepperBase : public TimeStepper
{
public:
  TimeSequenceStepperBase(const InputParameters & parameters);

  void setupSequence(const std::vector<Real> & times);

  virtual void init() override {}
  virtual void step() override;

protected:
  virtual Real computeInitialDT() override;
  virtual Real computeDT() override;
  virtual Real computeFailedDT() override;

  /// the step that the time stepper is currently at
  unsigned int & _current_step;

  /// stores the sequence of time points
  std::vector<Real> & _time_sequence;
};

#endif // TIMESEQUENCESTEPPERBASE_H

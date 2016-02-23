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

#include "TimeStepper.h"

class TimeSequenceStepper;

template<>
InputParameters validParams<TimeSequenceStepper>();

/**
 * Solves the PDEs at a sequence of given time points.
 * Adjusts the time sequence vector according to Transient start_time and end_time.
 */
class TimeSequenceStepper : public TimeStepper
{
public:
  TimeSequenceStepper(const InputParameters & parameters);
  virtual void init();
  virtual void step();

protected:
  virtual Real computeInitialDT();
  virtual Real computeDT();
  virtual Real computeFailedDT();

  /// the step that the time stepper is currently at
  unsigned int & _current_step;
  /// stores the sequence of time points
  std::vector<Real> _time_sequence;
};

#endif /* TimeSequenceStepper_H_ */

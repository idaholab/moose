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

#ifndef TIMELISTSTEPPERBASE_H
#define TIMELISTSTEPPERBASE_H

#include "Stepper.h"

class TimeListStepperBase;

template<>
InputParameters validParams<TimeListStepperBase>();

/**
 * Solves the PDEs at a sequence of given time points.
 * Adjusts the time sequence vector according to Transient start_time and end_time.
 */
class TimeListStepperBase : public Stepper
{
public:
  TimeListStepperBase(const InputParameters & parameters);

  virtual Real computeInitialDT() override;

  virtual Real computeDT() override;

  virtual Real computeFailedDT() override;

protected:
  /// Setup the list of times
  void setupList(const std::vector<Real> & times);

  /// If not coupled this will be max()
  const Real & _incoming_stepper_dt;

private:
  /// stores the sequence of time points.  Must be set by calling setupList()
  std::vector<Real> _time_list;
};

#endif //TIMELISTSTEPPERBASE_H

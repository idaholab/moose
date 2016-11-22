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

#ifndef TIMELISTSTEPPER_H
#define TIMELISTSTEPPER_H

#include "TimeListStepperBase.h"

class TimeListStepper;

template<>
InputParameters validParams<TimeListStepper>();

/**
 * Solves the PDEs at a list of time points given as a vector in the input file.
 * Adjusts the time list vector according to Transient start_time and end_time.
 */
class TimeListStepper : public TimeListStepperBase
{
public:
  TimeListStepper(const InputParameters & parameters);
};

#endif //TIMELISTSTEPPER_H

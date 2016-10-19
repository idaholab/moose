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

#include "TimeSequenceStepperBase.h"
#include "FEProblem.h"
#include "Transient.h"

template<>
InputParameters validParams<TimeSequenceStepperBase>()
{
  InputParameters params = validParams<TimeStepper>();
  return params;
}

TimeSequenceStepperBase::TimeSequenceStepperBase(const InputParameters & parameters) :
    TimeStepper(parameters),
    _time_sequence()
{
}

void
TimeSequenceStepperBase::setupSequence(const std::vector<Real> & times)
{
  // TODO: find a way for this function to access the startup time step if restarting to compare to previous sequence (which will need to be updated to be declareRestartable...
  _time_sequence = times;
}

Stepper*
TimeSequenceStepperBase::buildStepper()
{
  return new IfConvergedStepper(new FixedPointStepper(_time_sequence, 0), new GrowShrinkStepper(0.5, 1.0));
}

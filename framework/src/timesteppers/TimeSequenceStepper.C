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

#include "TimeSequenceStepper.h"

template <>
InputParameters
validParams<TimeSequenceStepper>()
{
  InputParameters params = validParams<TimeSequenceStepperBase>();
  params.addRequiredParam<std::vector<Real>>("time_sequence", "The values of t");
  params.addClassDescription("Solves the Transient problem at a sequence of given time points.");
  return params;
}

TimeSequenceStepper::TimeSequenceStepper(const InputParameters & parameters)
  : TimeSequenceStepperBase(parameters)
{
}

void
TimeSequenceStepper::init()
{
  setupSequence(getParam<std::vector<Real>>("time_sequence"));
}

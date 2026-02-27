//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TimeSequenceStepper.h"

registerMooseObject("MooseApp", TimeSequenceStepper);

InputParameters
TimeSequenceStepper::validParams()
{
  InputParameters params = TimeSequenceStepperBase::validParams();
  params.addRequiredParam<TimesName>("time_sequence", "The values of t");
  params.addClassDescription("Solves the Transient problem at a sequence of given time points.");
  return params;
}

TimeSequenceStepper::TimeSequenceStepper(const InputParameters & parameters)
  : TimeSequenceStepperBase(parameters), _times(getTimes("time_sequence"))
{
}

void
TimeSequenceStepper::init()
{
  setupSequence(_times.getTimes());
}

void
TimeSequenceStepper::step()
{
  TimeSequenceStepperBase::step();

  updateSequence();
}

void
TimeSequenceStepper::updateSequence()
{
  // Get the times again in case there are new ones
  const auto & times = _times.getTimes();
  if (_times.isDynamicTimeSequence())
    if (!std::includes(_time_sequence.begin(), _time_sequence.end(), times.begin(), times.end()))
      setupSequence(times);
}

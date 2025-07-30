//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TimeSequenceFromTimes.h"
#include "Times.h"

registerMooseObject("MooseApp", TimeSequenceFromTimes);

InputParameters
TimeSequenceFromTimes::validParams()
{
  InputParameters params = TimeSequenceStepperBase::validParams();
  params.addRequiredParam<TimesName>(
      "times", "The name of the Times object containing the times to hit during the simulation");
  params.addClassDescription("Solves the Transient problem at a sequence of time points taken from "
                             "a specified Times object.");
  return params;
}

TimeSequenceFromTimes::TimeSequenceFromTimes(const InputParameters & parameters)
  : TimeSequenceStepperBase(parameters),
    _times(_fe_problem.getUserObject<Times>(getParam<TimesName>("times"))),
    _time_points(_times.getTimes())
{
}

void
TimeSequenceFromTimes::init()
{
  setupSequence(_time_points);
}

void
TimeSequenceFromTimes::updateTimeSequence()
{
  resetSequence();
  setupSequence(_time_points);
}

Real
TimeSequenceFromTimes::computeDT()
{
  if (_times.isDynamicTimeSequence())
    updateTimeSequence();

  return TimeSequenceStepperBase::computeDT();
}

Real
TimeSequenceFromTimes::getNextTimeInSequence()
{
  if (_times.isDynamicTimeSequence())
    updateTimeSequence();
  return TimeSequenceStepperBase::getNextTimeInSequence();
}

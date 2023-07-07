//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
    _times(_fe_problem.getUserObject<Times>(getParam<TimesName>("times")))
{
  // If they are available, initialize
  const auto & times = _times.getTimes();
  setupSequence(times);
}

void
TimeSequenceFromTimes::step()
{
  // Get the times again in case there are new ones
  const auto & times = _times.getTimes();
  setupSequence(times);

  TimeSequenceStepperBase::step();
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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

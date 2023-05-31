//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestSourceStepper.h"

registerMooseObject("MooseTestApp", TestSourceStepper);

InputParameters
TestSourceStepper::validParams()
{
  InputParameters params = TimeStepper::validParams();
  params.addRequiredParam<Real>("dt", "Size of the time step");

  params.addClassDescription("A Timestepper that used to test rejectStep with CompositionDT");

  return params;
}

TestSourceStepper::TestSourceStepper(const InputParameters & parameters)
  : TimeStepper(parameters), _dt(getParam<Real>("dt"))
{
}

void
TestSourceStepper::rejectStep()
{
  TimeStepper::rejectStep();
  mooseWarning("rejectStep() calls from TestSourceStepper");
}

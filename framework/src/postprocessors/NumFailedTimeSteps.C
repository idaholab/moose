//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NumFailedTimeSteps.h"
#include "Transient.h"

registerMooseObject("MooseApp", NumFailedTimeSteps);

InputParameters
NumFailedTimeSteps::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription("Collects the number of failed time steps from the time stepper.");
  return params;
}

NumFailedTimeSteps::NumFailedTimeSteps(const InputParameters & parameters)
  : GeneralPostprocessor(parameters)
{
  if (_subproblem.isTransient())
  {
    _timestepper = dynamic_cast<TransientBase *>(_app.getExecutioner())->getTimeStepper();
    if (!_timestepper)
      mooseError("No user-specified time stepper in Executioner block");
  }
  else
    mooseError("Problem is not transient, and will not calculate timesteps.");
}

Real
NumFailedTimeSteps::getValue() const
{
  return _timestepper->numFailures();
}

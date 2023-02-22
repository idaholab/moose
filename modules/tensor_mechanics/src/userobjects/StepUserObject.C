//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StepUserObject.h"
#include <limits>

registerMooseObject("TensorMechanicsApp", StepUserObject);

InputParameters
StepUserObject::validParams()
{
  InputParameters params = GeneralUserObject::validParams();

  params.addRequiredParam<std::vector<Real>>(
      "times",
      "The times at which the objects are to be enabled/disabled at the beginning of the steps.");
  params.addRequiredParam<unsigned int>(
      "number_steps",
      "The number of loading steps defined for the simulation. The input 'times' "
      "must have the same size as the number of steps.");

  return params;
}

StepUserObject::StepUserObject(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _times(getParam<std::vector<Real>>("times")),
    _number_steps(getParam<unsigned int>("number_steps"))
{
  if (_times.size() != _number_steps)
    paramError("number_steps",
               "The number of steps does not match the times vector that was provided to this user "
               "object");
}

Real
StepUserObject::getStartTime(const unsigned int & step) const
{
  if (_times.size() < step)
    mooseError("StepUserObject was called with a wrong step number");

  return _times[step];
}

Real
StepUserObject::getEndTime(const unsigned int & step) const
{
  Real end_time(0);

  if (_times.size() > step + 1)
    end_time = _times[step + 1];
  else if (_times.size() == step + 1)
    end_time = std::numeric_limits<double>::max();
  else
    mooseError("StepUserObject was called with a wrong step number");

  return end_time;
}

unsigned int
StepUserObject::getStep(const Real & time) const
{
  int which_step = 0;

  for (const auto i : index_range(_times))
  {
    if (i == _times.size())
      break;

    which_step = i;

    if (time >= _times[i] && time < _times[i + 1])
      return which_step;
  }

  return which_step;
}

void
StepUserObject::initialize()
{
}
void
StepUserObject::execute()
{
}
void
StepUserObject::finalize()
{
}

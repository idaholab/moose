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
#include <algorithm> // std::is_sorted, std::prev_permutation

registerMooseObject("TensorMechanicsApp", StepUserObject);

InputParameters
StepUserObject::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredParam<std::vector<Real>>(
      "step_start_times",
      "The beginning of step times. The number of steps is inferred from the number of times. One "
      "step is defined by its start time; and its end time is taken from the start time of the "
      "next step (if it exists). This list needs to be in ascending value order.");

  return params;
}

StepUserObject::StepUserObject(const InputParameters & parameters)
  : GeneralUserObject(parameters), _times(getParam<std::vector<Real>>("step_start_times"))
{
  if (!std::is_sorted(_times.begin(), _times.end()))
    paramError("step_start_times",
               "start times for StepUserObject are not provided in ascending order. Please revise "
               "your input.");
}

Real
StepUserObject::getStartTime(const unsigned int & step) const
{
  if (_times.size() <= step)
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
    if (i + 1 == _times.size())
      return i;

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

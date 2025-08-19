//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ControllableInputTimes.h"

registerMooseObject("MooseApp", ControllableInputTimes);

InputParameters
ControllableInputTimes::validParams()
{
  InputParameters params = InputTimes::validParams();
  params.addClassDescription("Times set directly from a user parameter in the input file");
  params.addRequiredParam<Real>("next_time", "Time to store in the times vector");
  params.declareControllable("next_time");

  // Times are known for all processes already
  params.set<bool>("auto_broadcast") = false;
  params.makeParamNotRequired("times");

  return params;
}

ControllableInputTimes::ControllableInputTimes(const InputParameters & parameters)
  : InputTimes(parameters), _next_time(getParam<Real>("next_time"))
{
  // Initialize times array
  _times.push_back(0);
}

void
ControllableInputTimes::initialize()
{
  std::set<Real> times_set;
  if (_input_times.size())
    for (Real & t : _input_times)
      times_set.insert(t);

  times_set.insert(_next_time);

  for (Real t : times_set)
    _times.push_back(t);
}

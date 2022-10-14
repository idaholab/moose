//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SlowProblem.h"

#include "MooseApp.h"

#include <thread>
#include <chrono>

registerMooseObject("MooseTestApp", SlowProblem);

InputParameters
SlowProblem::validParams()
{
  InputParameters params = FEProblem::validParams();
  params.addRequiredParam<std::vector<Real>>("seconds_to_sleep", "The number of seconds to sleep");
  return params;
}

SlowProblem::SlowProblem(const InputParameters & params)
  : FEProblem(params), _seconds_to_sleep(getParam<std::vector<Real>>("seconds_to_sleep"))
{
  if (_seconds_to_sleep.empty())
    paramError("seconds_to_sleep", "Vector cannot be empty.");
}

void
SlowProblem::solve(unsigned int)
{
  {
    const Real delay = _t_step <= 0 ? _seconds_to_sleep.front()
                                    : (_t_step > static_cast<int>(_seconds_to_sleep.size())
                                           ? _seconds_to_sleep.back()
                                           : _seconds_to_sleep[_t_step - 1]);

    TIME_SECTION("slow", 1, "Testing Slowness");
    std::this_thread::sleep_for(std::chrono::duration<Real>(delay));
  }

  FEProblem::solve();
}

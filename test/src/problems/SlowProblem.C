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

  // Parameters to print perf graph live print interactions
  params.addParam<bool>(
      "print_during_section", false, "Whether to nest a console print inside the timed section");
  params.addParam<bool>(
      "nest_inside_section", false, "Whether to nest another section inside the timed section");

  return params;
}

SlowProblem::SlowProblem(const InputParameters & params)
  : FEProblem(params),
    _seconds_to_sleep(getParam<std::vector<Real>>("seconds_to_sleep")),
    _nested_print(getParam<bool>("print_during_section")),
    _nested_section(getParam<bool>("nest_inside_section"))
{
  if (_seconds_to_sleep.empty())
    paramError("seconds_to_sleep", "Vector cannot be empty.");
}

void
SlowProblem::solve(unsigned int)
{
  {
    const Real delay = getDelay();

    if (!_nested_print && !_nested_section)
    {
      TIME_SECTION("slow", 1, "Testing Slowness");
      std::this_thread::sleep_for(std::chrono::duration<Real>(delay));
    }
    else if (_nested_print)
    {
      TIME_SECTION("slow", 1, "Testing Slowness");
      _console << "Timed section just started, printing something though" << std::endl;
      std::this_thread::sleep_for(std::chrono::duration<Real>(delay));
    }
    else if (_nested_section)
    {
      TIME_SECTION("slow", 1, "Testing Slowness");
      otherTimedSection();
      std::this_thread::sleep_for(std::chrono::duration<Real>(delay));
    }
  }

  FEProblem::solve();
}

void
SlowProblem::otherTimedSection() const
{
  Real delay = getDelay();
  TIME_SECTION("slowish", 2, "Testing Nested Slowness");
  std::this_thread::sleep_for(std::chrono::duration<Real>(delay));
}

Real
SlowProblem::getDelay() const
{
  return _t_step <= 0 ? _seconds_to_sleep.front()
                      : (_t_step > static_cast<int>(_seconds_to_sleep.size())
                             ? _seconds_to_sleep.back()
                             : _seconds_to_sleep[_t_step - 1]);
}

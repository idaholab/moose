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
  params.addRequiredParam<unsigned int>("seconds_to_sleep", "The number of seconds to sleep");
  return params;
}

SlowProblem::SlowProblem(const InputParameters & params)
  : FEProblem(params), _seconds_to_sleep(getParam<unsigned int>("seconds_to_sleep"))
{
}

void
SlowProblem::solve()
{
  {
    TIME_SECTION("slow", 1, "Testing Slowness");
    std::this_thread::sleep_for(std::chrono::seconds(_seconds_to_sleep));
  }

  FEProblem::solve();
}

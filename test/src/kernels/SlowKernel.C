//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SlowKernel.h"

#include <thread>
#include <limits>
#include <chrono>

registerMooseObject("MooseTestApp", SlowKernel);

InputParameters
SlowKernel::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addRequiredParam<std::vector<Real>>(
      "step_delay", "List of delays this Kernel causes in seconds for each timestep.");

  return params;
}

SlowKernel::SlowKernel(const InputParameters & parameters)
  : Kernel(parameters),
    _last_step(std::numeric_limits<int>::min()),
    _step_delay(getParam<std::vector<Real>>("step_delay"))
{
}

Real
SlowKernel::computeQpResidual()
{
  if (_last_step != _t_step)
  {
    _last_step = _t_step;

    const Real delay = _t_step > 0 && _t_step <= static_cast<int>(_step_delay.size())
                           ? _step_delay[_t_step - 1]
                           : 0.0;
    std::this_thread::sleep_for(std::chrono::duration<Real>(delay));
  }

  return 0.0;
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TransientHalf.h"

// Moose includes

registerMooseObject("ExampleApp", TransientHalf);

InputParameters
TransientHalf::validParams()
{
  InputParameters params = TimeStepper::validParams();
  params.addParam<Real>("dt", 1., "The initial time step size.");
  params.addParam<Real>("ratio", 0.5, "The ratio used to calculate the next timestep");
  params.addParam<Real>("min_dt", 0.01, "The smallest timestep we will allow");
  return params;
}

TransientHalf::TransientHalf(const InputParameters & parameters)
  : TimeStepper(parameters), _ratio(getParam<Real>("ratio")), _min_dt(getParam<Real>("min_dt"))
{
}

Real
TransientHalf::computeInitialDT()
{
  return getParam<Real>("dt");
}

Real
TransientHalf::computeDT()
{
  /**
   * We won't grow timesteps with this example so if the ratio > 1.0 we'll just
   * leave current_dt alone.
   */
  if (_ratio < 1.0)
    // Shrink our timestep by the specified ratio or return the min if it's too small
    return std::max(getCurrentDT() * _ratio, _min_dt);
  else
    return getCurrentDT();
}

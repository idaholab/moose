/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "TransientHalf.h"

//Moose includes

template<>
InputParameters validParams<TransientHalf>()
{
  InputParameters params = validParams<TimeStepper>();
  params.addParam<Real>("dt", 1., "The initial time step size.");
  params.addParam<Real>("ratio", 0.5, "The ratio used to calculate the next timestep");
  params.addParam<Real>("min_dt", 0.01, "The smallest timestep we will allow");
  return params;
}

TransientHalf::TransientHalf(const std::string & name, InputParameters parameters) :
    TimeStepper(name, parameters),
    _ratio(getParam<Real>("ratio")),
    _min_dt(getParam<Real>("min_dt"))
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

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

#include "TimestepSetupFunction.h"

template<>
InputParameters validParams<TimestepSetupFunction>()
{
  InputParameters params = validParams<Function>();
  return params;
}

TimestepSetupFunction::TimestepSetupFunction(const std::string & name, InputParameters parameters) :
    Function(name, parameters),
    _local_timestep(0)
{}

void
TimestepSetupFunction::timestepSetup()
{
  _local_timestep++;
}

Real
TimestepSetupFunction::value(Real /*t*/, const Point & /*p*/)
{
  return _local_timestep;
}

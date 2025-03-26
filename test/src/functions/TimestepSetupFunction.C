//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TimestepSetupFunction.h"

registerMooseObject("MooseTestApp", TimestepSetupFunction);

InputParameters
TimestepSetupFunction::validParams()
{
  InputParameters params = Function::validParams();
  return params;
}

TimestepSetupFunction::TimestepSetupFunction(const InputParameters & parameters)
  : Function(parameters), _local_timestep(-1)
{
}

void
TimestepSetupFunction::timestepSetup()
{
  _local_timestep = _t_step;
}

Real
TimestepSetupFunction::value(Real /*t*/, const Point & /*p*/) const
{
  return _local_timestep;
}

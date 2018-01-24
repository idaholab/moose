//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "UniformDistribution.h"

template <>
InputParameters
validParams<UniformDistribution>()
{
  InputParameters params = validParams<Distribution>();
  params.addClassDescription("Continuous uniform distribution.");
  params.addParam<Real>("lower_bound", 0.0, "Distribution lower bound");
  params.addParam<Real>("upper_bound", 1.0, "Distribution upper bound");
  return params;
}

UniformDistribution::UniformDistribution(const InputParameters & parameters)
  : Distribution(parameters),
    _lower_bound(getParam<Real>("lower_bound")),
    _upper_bound(getParam<Real>("upper_bound"))
{
  if (_lower_bound >= _upper_bound)
    mooseError("The lower bound is larger than the upper bound!");
}

Real
UniformDistribution::pdf(const Real & x)
{
  if (x < _lower_bound || x > _upper_bound)
    return 0.0;
  else
    return 1.0 / (_upper_bound - _lower_bound);
}

Real
UniformDistribution::cdf(const Real & x)
{
  if (x < _lower_bound)
    return 0.0;
  else if (x > _upper_bound)
    return 1.0;
  else
    return (x - _lower_bound) / (_upper_bound - _lower_bound);
}

Real
UniformDistribution::quantile(const Real & y)
{
  if (y < 0 || y > 1)
    mooseError("The cdf_value provided is out of range 0 to 1.");
  else
    return y * (_upper_bound - _lower_bound) + _lower_bound;
}

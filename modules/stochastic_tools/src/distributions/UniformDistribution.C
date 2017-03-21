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

UniformDistribution::~UniformDistribution() {}

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
UniformDistribution::inverseCdf(const Real & y)
{
  if (y < 0 || y > 1)
    mooseError("The cdf_value provided is out of range 0 to 1.");
  else
    return y * (_upper_bound - _lower_bound) + _lower_bound;
}

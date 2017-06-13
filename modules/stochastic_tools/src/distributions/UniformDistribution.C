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
    BasicUniformDistribution(getParam<Real>("lower_bound"), getParam<Real>("upper_bound"))
{
}

UniformDistribution::~UniformDistribution() {}

Real
UniformDistribution::pdf(const Real & x)
{
  return BasicUniformDistribution::pdf(x);
}

Real
UniformDistribution::cdf(const Real & x)
{
  return BasicUniformDistribution::cdf(x);
}

Real
UniformDistribution::inverseCdf(const Real & y)
{
  if (y < 0 || y > 1)
    mooseError("The cdf_value provided is out of range 0 to 1.");
  else
    return BasicUniformDistribution::inverseCdf(y);
}

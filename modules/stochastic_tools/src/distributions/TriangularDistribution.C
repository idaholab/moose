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

#include "TriangularDistribution.h"

template <>
InputParameters
validParams<TriangularDistribution>()
{
  InputParameters params = validParams<Distribution>();
  params.addClassDescription("Continuous uniform distribution.");
  params.addParam<Real>("lower_bound", "Distribution lower bound");
  params.addParam<Real>("upper_bound", "Distribution upper bound");
  params.addParam<Real>("x_peak", "Distribution central peak");
  return params;
}

TriangularDistribution::TriangularDistribution(const InputParameters & parameters)
  : Distribution(parameters),
    BasicTriangularDistribution(getParam<Real>("x_peak"),
                                getParam<Real>("lower_bound"),
                                getParam<Real>("upper_bound"))
{
}

TriangularDistribution::~TriangularDistribution() {}

Real
TriangularDistribution::pdf(const Real & x)
{
  return BasicTriangularDistribution::pdf(x);
}

Real
TriangularDistribution::cdf(const Real & x)
{
  return BasicTriangularDistribution::cdf(x);
}

Real
TriangularDistribution::inverseCdf(const Real & y)
{
  if (y < 0 || y > 1)
    mooseError("The cdf_value provided is out of range 0 to 1.");
  else
    return BasicTriangularDistribution::inverseCdf(y);
}



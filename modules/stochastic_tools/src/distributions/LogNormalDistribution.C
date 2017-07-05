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

#include "LogNormalDistribution.h"

template <>
InputParameters
validParams<LogNormalDistribution>()
{
  InputParameters params = validParams<Distribution>();
  params.addClassDescription("Continuous lognormal distribution.");
  params.addParam<Real>("lower_bound", 0.0, "Distribution lower bound");
  params.addParam<Real>(
      "upper_bound", std::numeric_limits<Real>::max(), "Distribution upper bound");
  params.addParam<Real>("mu", 0.0, "Distribution mu");
  params.addParam<Real>("sigma", 1.0, "Distribution sigma");
  return params;
}

LogNormalDistribution::LogNormalDistribution(const InputParameters & parameters)
  : Distribution(parameters),
    BasicLogNormalDistribution(getParam<Real>("mu"),
                               getParam<Real>("sigma"),
                               getParam<Real>("lower_bound"),
                               getParam<Real>("upper_bound"),
                               getParam<Real>("lower_bound"))
{
}

LogNormalDistribution::~LogNormalDistribution() {}

Real
LogNormalDistribution::pdf(const Real & x)
{
  return BasicLogNormalDistribution::pdf(x);
}

Real
LogNormalDistribution::cdf(const Real & x)
{
  return BasicLogNormalDistribution::cdf(x);
}

Real
LogNormalDistribution::inverseCdf(const Real & y)
{
  if (y < 0 || y > 1)
    mooseError("The cdf_value provided is out of range 0 to 1.");
  else
    return BasicLogNormalDistribution::inverseCdf(y);
}

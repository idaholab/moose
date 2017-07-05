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

#include "NormalDistribution.h"

template <>
InputParameters
validParams<NormalDistribution>()
{
  InputParameters params = validParams<Distribution>();
  params.addClassDescription("Continuous normal distribution.");
  params.addParam<Real>(
      "lower_bound", -std::numeric_limits<Real>::max(), "Distribution lower bound");
  params.addParam<Real>(
      "upper_bound", std::numeric_limits<Real>::max(), "Distribution upper bound");
  params.addParam<Real>("mu", 0.0, "Distribution mu");
  params.addParam<Real>("sigma", 1.0, "Distribution sigma");
  return params;
}

NormalDistribution::NormalDistribution(const InputParameters & parameters)
  : Distribution(parameters),
    BasicNormalDistribution(getParam<Real>("mu"),
                            getParam<Real>("sigma"),
                            getParam<Real>("lower_bound"),
                            getParam<Real>("upper_bound"))
{
}

NormalDistribution::~NormalDistribution() {}

Real
NormalDistribution::pdf(const Real & x)
{
  return BasicNormalDistribution::pdf(x);
}

Real
NormalDistribution::cdf(const Real & x)
{
  return BasicNormalDistribution::cdf(x);
}

Real
NormalDistribution::inverseCdf(const Real & y)
{
  if (y < 0 || y > 1)
    mooseError("The cdf_value provided is out of range 0 to 1.");
  else
    return BasicNormalDistribution::inverseCdf(y);
}

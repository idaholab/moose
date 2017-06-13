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

#include "WeibullDistribution.h"

template <>
InputParameters
validParams<WeibullDistribution>()
{
  InputParameters params = validParams<Distribution>();
  params.addClassDescription("Continuous uniform distribution.");
  params.addParam<Real>("lower_bound", 0.0, "Distribution lower bound");
  params.addParam<Real>(
      "upper_bound", std::numeric_limits<Real>::max(), "Distribution upper bound");
  params.addParam<Real>("k", "Distribution shape factor");
  params.addParam<Real>("lambda", "Distribution scale factor");
  params.addParam<Real>("low", "Distribution lower domain scale");
  return params;
}

WeibullDistribution::WeibullDistribution(const InputParameters & parameters)
  : Distribution(parameters),
    BasicWeibullDistribution(getParam<Real>("k"),
                             getParam<Real>("lambda"),
                             getParam<Real>("lower_bound"),
                             getParam<Real>("upper_bound"),
                             getParam<Real>("low"))
{
}

WeibullDistribution::~WeibullDistribution() {}

Real
WeibullDistribution::pdf(const Real & x)
{
  return BasicWeibullDistribution::pdf(x);
}

Real
WeibullDistribution::cdf(const Real & x)
{
  return BasicWeibullDistribution::cdf(x);
}

Real
WeibullDistribution::inverseCdf(const Real & y)
{
  if (y < 0 || y > 1)
    mooseError("The cdf_value provided is out of range 0 to 1.");
  else
    return BasicWeibullDistribution::inverseCdf(y);
}

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

#include "BetaDistribution.h"

template <>
InputParameters
validParams<BetaDistribution>()
{
  InputParameters params = validParams<Distribution>();
  params.addClassDescription("Continuous beta distribution.");
  params.addParam<Real>("lower_bound", 0.0, "Distribution lower bound");
  params.addParam<Real>("upper_bound", 1.0, "Distribution upper bound");
  params.addParam<Real>("alpha", "Distribution alpha coefficient");
  params.addParam<Real>("beta", "Distribution beta coefficient");
  params.addParam<Real>("scale", 1.0, "Distribution scale factor");
  return params;
}

BetaDistribution::BetaDistribution(const InputParameters & parameters)
  : Distribution(parameters),
    BasicBetaDistribution(getParam<Real>("alpha"),
                          getParam<Real>("beta"),
                          getParam<Real>("scale"),
                          getParam<Real>("lower_bound"),
                          getParam<Real>("upper_bound"),
                          getParam<Real>("lower_bound") // this is done on purpose
                          )
{
}

BetaDistribution::~BetaDistribution() {}

Real
BetaDistribution::pdf(const Real & x)
{
  return BasicBetaDistribution::pdf(x);
}

Real
BetaDistribution::cdf(const Real & x)
{
  return BasicBetaDistribution::cdf(x);
}

Real
BetaDistribution::inverseCdf(const Real & y)
{
  if (y < 0 || y > 1)
    mooseError("The cdf_value provided is out of range 0 to 1.");
  else
    return BasicBetaDistribution::inverseCdf(y);
}

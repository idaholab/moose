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
  params.addParam<Real>("alpha", 0.0, "Distribution lower bound");
  params.addParam<Real>("beta", 1.0, "Distribution upper bound");
  params.addParam<Real>("scale", 0.0, "Distribution lower bound");
  params.addParam<Real>("low", 1.0, "Distribution upper bound");
  return params;
}

BetaDistribution::BetaDistribution(const InputParameters & parameters)
  : Distribution(parameters),
    BasicBetaDistribution(getParam<Real>("lower_bound"),
                          getParam<Real>("upper_bound"),
                          getParam<Real>("upper_bound"),
                          getParam<Real>("upper_bound"),
                          getParam<Real>("upper_bound"),
                          getParam<Real>("lower_bound"),
                          getParam<Real>("upper_bound")
                          
                          
                          
                          )
{
}


double alpha, double beta, double scale, double x_min, double x_max, double low


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



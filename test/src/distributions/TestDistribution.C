//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestDistribution.h"

registerMooseObject("MooseTestApp", TestDistribution);

InputParameters
TestDistribution::validParams()
{
  InputParameters params = Distribution::validParams();
  return params;
}

TestDistribution::TestDistribution(const InputParameters & parameters) : Distribution(parameters) {}

Real
TestDistribution::pdf(const Real & /* x */) const
{
  return 1.0;
}

Real
TestDistribution::cdf(const Real & x) const
{
  return x;
}

Real
TestDistribution::quantile(const Real & y) const
{
  return y;
}

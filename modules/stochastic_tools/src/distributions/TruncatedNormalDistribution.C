//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TruncatedNormalDistribution.h"

registerMooseObject("StochasticToolsApp", TruncatedNormalDistribution);

template <>
InputParameters
validParams<TruncatedNormalDistribution>()
{
  InputParameters params = validParams<NormalDistribution>();
  params.addClassDescription("Truncated normal distribution");
  params.addParam<Real>(
      "lower_bound", -std::numeric_limits<Real>::max(), "Lower bound of the distribution ");
  params.addParam<Real>(
      "upper_bound", std::numeric_limits<Real>::max(), "Upper bound of the distribution ");
  return params;
}

TruncatedNormalDistribution::TruncatedNormalDistribution(const InputParameters & parameters)
  : NormalDistribution(parameters),
    _lower_bound(getParam<Real>("lower_bound")),
    _upper_bound(getParam<Real>("upper_bound"))
{
  if (_lower_bound >= _upper_bound)
    mooseError("lower_bound in truncated normal distribution must be less than upper_bound.");
}

Real
TruncatedNormalDistribution::pdf(const Real & x)
{
  if (x <= _lower_bound || x >= _upper_bound)
    return 0.0;
  else
    return (NormalDistribution::pdf(x)) /
           (NormalDistribution::cdf(_upper_bound) - NormalDistribution::cdf(_lower_bound));
}

Real
TruncatedNormalDistribution::cdf(const Real & x)
{
  if (x <= _lower_bound || x >= _upper_bound)
    return 0.0;
  else
    return (NormalDistribution::cdf(x) - NormalDistribution::cdf(_lower_bound)) /
           (NormalDistribution::cdf(_upper_bound) - NormalDistribution::cdf(_lower_bound));
}

Real
TruncatedNormalDistribution::quantile(const Real & p)
{
  return NormalDistribution::quantile(
      NormalDistribution::cdf(_lower_bound) +
      p * (NormalDistribution::cdf(_upper_bound) - NormalDistribution::cdf(_lower_bound)));
}

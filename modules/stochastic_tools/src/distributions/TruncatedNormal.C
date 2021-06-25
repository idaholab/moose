//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TruncatedNormal.h"

registerMooseObject("StochasticToolsApp", TruncatedNormal);

InputParameters
TruncatedNormal::validParams()
{
  InputParameters params = Normal::validParams();
  params.addClassDescription("Truncated normal distribution");
  params.addParam<Real>(
      "lower_bound", -std::numeric_limits<Real>::max(), "Lower bound of the distribution ");
  params.addParam<Real>(
      "upper_bound", std::numeric_limits<Real>::max(), "Upper bound of the distribution ");
  return params;
}

TruncatedNormal::TruncatedNormal(const InputParameters & parameters)
  : Normal(parameters),
    _lower_bound(getParam<Real>("lower_bound")),
    _upper_bound(getParam<Real>("upper_bound"))
{
  if (_lower_bound >= _upper_bound)
    mooseError("lower_bound in truncated normal distribution must be less than upper_bound.");
}

Real
TruncatedNormal::pdf(const Real & x,
                     const Real & mean,
                     const Real & std_dev,
                     const Real & lower_bound,
                     const Real & upper_bound)
{
  if (x <= lower_bound || x >= upper_bound)
    return 0.0;
  else
    return (Normal::pdf(x, mean, std_dev)) /
           (Normal::cdf(upper_bound, mean, std_dev) - Normal::cdf(lower_bound, mean, std_dev));
}

Real
TruncatedNormal::cdf(const Real & x,
                     const Real & mean,
                     const Real & std_dev,
                     const Real & lower_bound,
                     const Real & upper_bound)
{

  if (x <= lower_bound || x >= upper_bound)
    return 0.0;
  else
    return (Normal::cdf(x, mean, std_dev) - Normal::cdf(lower_bound, mean, std_dev)) /
           (Normal::cdf(upper_bound, mean, std_dev) - Normal::cdf(lower_bound, mean, std_dev));
}

Real
TruncatedNormal::quantile(const Real & p,
                          const Real & mean,
                          const Real & std_dev,
                          const Real & lower_bound,
                          const Real & upper_bound)
{
  return Normal::quantile(
      Normal::cdf(lower_bound, mean, std_dev) +
          p * (Normal::cdf(upper_bound, mean, std_dev) - Normal::cdf(lower_bound, mean, std_dev)),
      mean,
      std_dev);
}

Real
TruncatedNormal::pdf(const Real & x) const
{
  return pdf(x, _mean, _standard_deviation, _lower_bound, _upper_bound);
}

Real
TruncatedNormal::cdf(const Real & x) const
{
  return cdf(x, _mean, _standard_deviation, _lower_bound, _upper_bound);
}

Real
TruncatedNormal::quantile(const Real & p) const
{
  return quantile(p, _mean, _standard_deviation, _lower_bound, _upper_bound);
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NormalDistribution.h"
#include "math.h"
#include "libmesh/utility.h"

registerMooseObject("StochasticToolsApp", NormalDistribution);

template <>
InputParameters
validParams<NormalDistribution>()
{
  InputParameters params = validParams<Distribution>();
  params.addClassDescription("Normal distribution");
  params.addRequiredParam<Real>("mean", "Mean (or expectation) of the distribution.");
  params.addRequiredRangeCheckedParam<Real>(
      "standard_deviation", "standard_deviation > 0", "Standard deviation of the distribution ");
  return params;
}

NormalDistribution::NormalDistribution(const InputParameters & parameters)
  : Distribution(parameters),
    _mean(getParam<Real>("mean")),
    _standard_deviation(getParam<Real>("standard_deviation"))
{
}
Real
NormalDistribution::pdf(const Real & x, const Real & mean, const Real & std_dev) const
{
  return 1.0 / (std_dev * std::sqrt(2.0 * M_PI)) *
         std::exp(-0.5 * Utility::pow<2>((x - mean) / std_dev));
}

Real
NormalDistribution::cdf(const Real & x, const Real & mean, const Real & std_dev) const
{
  return 0.5 * (1.0 + std::erf((x - mean) / (std_dev * std::sqrt(2.0))));
}

Real
NormalDistribution::quantile(const Real & p, const Real & mean, const Real & std_dev) const
{
  Real x = (p < 0.5 ? p : 1.0 - p);
  Real y = std::sqrt(-2.0 * std::log(x));
  Real sgn = (p - 0.5 < 0.0 ? -1.0 : 1.0);
  Real Zp = sgn * (y + (_a[0] + _a[1] * y + _a[2] * Utility::pow<2>(y) +
                        _a[3] * Utility::pow<3>(y) + _a[4] * Utility::pow<4>(y)) /
                           (_b[0] + _b[1] * y + _b[2] * Utility::pow<2>(y) +
                            _b[3] * Utility::pow<3>(y) + _b[4] * Utility::pow<4>(y)));
  return Zp * std_dev + mean;
}

Real
NormalDistribution::pdf(const Real & x) const
{
  return pdf(x, _mean, _standard_deviation);
}

Real
NormalDistribution::cdf(const Real & x) const
{
  return cdf(x, _mean, _standard_deviation);
}

Real
NormalDistribution::quantile(const Real & p) const
{
  return quantile(p, _mean, _standard_deviation);
}

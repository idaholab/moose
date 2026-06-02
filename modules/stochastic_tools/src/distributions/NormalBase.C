//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NormalBase.h"
#include "math.h"
#include "libmesh/utility.h"

const std::array<Real, 6> NormalBase::_a = {
    {-0.322232431088, -1.0, -0.342242088547, -0.0204231210245, -0.0000453642210148}};

const std::array<Real, 6> NormalBase::_b = {
    {0.099348462606, 0.588581570495, 0.531103462366, 0.10353775285, 0.0038560700634}};

NormalBase::NormalBase(const Real mean, const Real standard_deviation)
  : _mean(mean), _standard_deviation(standard_deviation)
{
}

Real
NormalBase::pdf(const Real & x, const Real & mean, const Real & std_dev)
{
  return 1.0 / (std_dev * std::sqrt(2.0 * M_PI)) *
         std::exp(-0.5 * Utility::pow<2>((x - mean) / std_dev));
}

Real
NormalBase::cdf(const Real & x, const Real & mean, const Real & std_dev)
{
  return 0.5 * (1.0 + std::erf((x - mean) / (std_dev * std::sqrt(2.0))));
}

Real
NormalBase::quantile(const Real & p, const Real & mean, const Real & std_dev)
{
  const Real x = (p < 0.5 ? p : 1.0 - p);
  const Real y = std::sqrt(-2.0 * std::log(x));
  const Real y2 = y * y;
  const Real y3 = y2 * y;
  const Real y4 = y3 * y;
  const Real sgn = (p - 0.5 < 0.0 ? -1.0 : 1.0);
  const Real Zp = sgn * (y + (_a[0] + _a[1] * y + _a[2] * y2 + _a[3] * y3 + _a[4] * y4) /
                                 (_b[0] + _b[1] * y + _b[2] * y2 + _b[3] * y3 + _b[4] * y4));
  return Zp * std_dev + mean;
}

Real
NormalBase::pdf(const Real & x) const
{
  return pdf(x, _mean, _standard_deviation);
}

Real
NormalBase::cdf(const Real & x) const
{
  return cdf(x, _mean, _standard_deviation);
}

Real
NormalBase::quantile(const Real & p) const
{
  return quantile(p, _mean, _standard_deviation);
}

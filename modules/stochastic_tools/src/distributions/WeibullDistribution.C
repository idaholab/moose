//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WeibullDistribution.h"
#include "math.h"
#include "libmesh/utility.h"

registerMooseObject("StochasticToolsApp", WeibullDistribution);

template <>
InputParameters
validParams<WeibullDistribution>()
{
  InputParameters params = validParams<Distribution>();
  params.addClassDescription("Three-parameter Weibull distribution.");
  params.addRequiredParam<Real>("location", "Location parameter (a or low)");
  params.addRequiredRangeCheckedParam<Real>("scale", "scale > 0", "Scale parameter (b or lambda)");
  params.addRequiredRangeCheckedParam<Real>("shape", "shape > 0", "Shape parameter (c or k)");
  return params;
}

WeibullDistribution::WeibullDistribution(const InputParameters & parameters)
  : Distribution(parameters),
    _a(getParam<Real>("location")),
    _b(getParam<Real>("scale")),
    _c(getParam<Real>("shape"))
{
}

Real
WeibullDistribution::pdf(const Real & x)
{
  if (x <= _a)
    return 0.0;
  else
  {
    const Real y = (x - _a) / _b;
    return _c / _b * std::pow(y, _c - 1.0) * std::exp(-std::pow(y, _c));
  }
}

Real
WeibullDistribution::cdf(const Real & x)
{
  if (x <= _a)
    return 0.0;
  else
  {
    const Real y = (x - _a) / _b;
    return 1.0 - std::exp(-std::pow(y, _c));
  }
}

Real
WeibullDistribution::quantile(const Real & p)
{
  return _a + _b * std::pow(-std::log(1 - p), 1.0 / _c);
}

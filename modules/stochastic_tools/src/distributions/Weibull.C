//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Weibull.h"
#include "math.h"
#include "libmesh/utility.h"

registerMooseObject("StochasticToolsApp", Weibull);

InputParameters
Weibull::validParams()
{
  InputParameters params = Distribution::validParams();
  params.addClassDescription("Three-parameter Weibull distribution.");
  params.addRequiredParam<Real>("location", "Location parameter (a or low)");
  params.addRequiredRangeCheckedParam<Real>("scale", "scale > 0", "Scale parameter (b or lambda)");
  params.addRequiredRangeCheckedParam<Real>("shape", "shape > 0", "Shape parameter (c or k)");
  return params;
}

Weibull::Weibull(const InputParameters & parameters)
  : Distribution(parameters),
    _a(getParam<Real>("location")),
    _b(getParam<Real>("scale")),
    _c(getParam<Real>("shape"))
{
}

Real
Weibull::pdf(const Real & x, const Real & location, const Real & scale, const Real & shape)
{
  if (x <= location)
    return 0.0;
  else
  {
    const Real y = (x - location) / scale;
    return shape / scale * std::pow(y, shape - 1.0) * std::exp(-std::pow(y, shape));
  }
}

Real
Weibull::cdf(const Real & x, const Real & location, const Real & scale, const Real & shape)
{
  if (x <= location)
    return 0.0;
  else
  {
    const Real y = (x - location) / scale;
    return 1.0 - std::exp(-std::pow(y, shape));
  }
}

Real
Weibull::quantile(const Real & p, const Real & location, const Real & scale, const Real & shape)
{
  return location + scale * std::pow(-std::log(1 - p), 1.0 / shape);
}

Real
Weibull::pdf(const Real & x) const
{
  return pdf(x, _a, _b, _c);
}

Real
Weibull::cdf(const Real & x) const
{
  return cdf(x, _a, _b, _c);
}

Real
Weibull::quantile(const Real & p) const
{
  return quantile(p, _a, _b, _c);
}

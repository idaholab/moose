//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Lognormal.h"
#include "Normal.h"
#include "math.h"
#include "libmesh/utility.h"

registerMooseObject("StochasticToolsApp", Lognormal);

InputParameters
Lognormal::validParams()
{
  InputParameters params = Distribution::validParams();
  params.addClassDescription("Lognormal distribution");
  params.addRequiredParam<Real>("location", "The Lognormal location parameter (m or mu).");
  params.addRequiredParam<Real>("scale", "The Lognormal scale parameter (s or sigma).");
  return params;
}

Lognormal::Lognormal(const InputParameters & parameters)
  : Distribution(parameters), _location(getParam<Real>("location")), _scale(getParam<Real>("scale"))
{
}

Real
Lognormal::pdf(const Real & x, const Real & location, const Real & scale)
{
  return 1.0 / (x * scale * std::sqrt(2.0 * M_PI)) *
         std::exp(-0.5 * Utility::pow<2>((std::log(x) - location) / scale));
}

Real
Lognormal::cdf(const Real & x, const Real & location, const Real & scale)
{
  return 0.5 * (1.0 + std::erf((std::log(x) - location) / (scale * std::sqrt(2.0))));
}

Real
Lognormal::quantile(const Real & p, const Real & location, const Real & scale)
{
  return std::exp(Normal::quantile(p, location, scale));
}

Real
Lognormal::pdf(const Real & x) const
{
  return pdf(x, _location, _scale);
}

Real
Lognormal::cdf(const Real & x) const
{
  return cdf(x, _location, _scale);
}

Real
Lognormal::quantile(const Real & p) const
{
  return quantile(p, _location, _scale);
}

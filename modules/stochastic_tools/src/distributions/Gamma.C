//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Gamma.h"
#include "math.h"
#include "libmesh/utility.h"

registerMooseObject("StochasticToolsApp", Gamma);

InputParameters
Gamma::validParams()
{
  InputParameters params = Distribution::validParams();
  params.addClassDescription("Gamma distribution");
  params.addRequiredRangeCheckedParam<Real>("shape", "shape > 0", "Shape parameter (k or alpha).");
  params.addRangeCheckedParam<Real>(
      "scale", 1.0, "scale > 0", "Scale parameter (theta or 1/beta).");
  return params;
}

Gamma::Gamma(const InputParameters & parameters)
  : Distribution(parameters), _alpha(getParam<Real>("shape")), _theta(getParam<Real>("scale"))
{
}

Real
Gamma::pdf(const Real & x, const Real & alpha, const Real & beta)
{
  if (x <= 0.0)
    return 0.0;
  else
    return std::pow(beta, alpha) * std::pow(x, alpha - 1.0) * std::exp(-beta * x) /
           std::tgamma(alpha);
}

Real
Gamma::cdf(const Real & x, const Real & alpha, const Real & beta)
{
  if (x <= 0.0)
    return 0.0;
  else
    return incompleteGamma(alpha, beta * x);
}

Real
Gamma::quantile(const Real & p, const Real & alpha, const Real & beta)
{
  return incompleteGammaInv(alpha, p) / beta;
}

Real
Gamma::pdf(const Real & x) const
{
  return pdf(x, _alpha, 1.0 / _theta);
}

Real
Gamma::cdf(const Real & x) const
{
  return cdf(x, _alpha, 1.0 / _theta);
}

Real
Gamma::quantile(const Real & p) const
{
  return quantile(p, _alpha, 1.0 / _theta);
}

Real
Gamma::incompleteGamma(const Real & a, const Real & x)
{
  const Real tol = 1e-14;
  const unsigned int max_iter = 1e6;
  const Real coef = std::pow(x, a) * std::exp(-x) / std::tgamma(a + 1.0);
  Real cn = 1.0;
  Real an = a;
  Real val = 1.0;
  for (unsigned int i = 0; i < max_iter; ++i)
  {
    an += 1.0;
    cn *= x / an;
    val += cn;
    if (std::abs(cn / val) < tol)
      return coef * val;
  }

  mooseAssert(false, "Could not compute incomplete gamma function.");
  return coef * val;
}

Real
Gamma::incompleteGammaInv(const Real & a, const Real & p)
{
  Real x = a > 1.0 ? a : std::pow(p * std::tgamma(a + 1.0), 1.0 / a);
  const Real scale = std::tgamma(a);
  const Real tol = 1e-14;
  const unsigned int max_iter = 1e6;
  Real f;
  Real df;
  for (unsigned int i = 0; i < max_iter; ++i)
  {
    f = incompleteGamma(a, x) - p;
    if (std::abs(f) < tol)
      return x;
    df = std::pow(x, a - 1.0) * std::exp(-x) / scale;
    x -= f / df;
  }

  mooseAssert(false, "Could not find inverse of incomplete gamma function.");
  return x;
}

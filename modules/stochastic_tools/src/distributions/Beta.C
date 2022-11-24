//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Beta.h"
#include "math.h"
#include "libmesh/utility.h"

registerMooseObject("StochasticToolsApp", Beta);

InputParameters
Beta::validParams()
{
  InputParameters params = Distribution::validParams();
  params.addClassDescription("Beta distribution");
  params.addRequiredRangeCheckedParam<Real>("alpha", "alpha > 0", "Shape parameter 1.");
  params.addRequiredRangeCheckedParam<Real>("beta", "beta > 0", "Shape parameter 2.");
  return params;
}

Beta::Beta(const InputParameters & parameters)
  : Distribution(parameters), _alpha(getParam<Real>("alpha")), _beta(getParam<Real>("beta"))
{
}

Real
Beta::pdf(const Real & x, const Real & alpha, const Real & beta)
{
  if (x <= 0.0 || x > 1.0)
    return 0.0;
  else
    return std::pow(x, alpha - 1.0) * std::pow(1.0 - x, beta - 1.0) / betaFunction(alpha, beta);
}

Real
Beta::cdf(const Real & x, const Real & alpha, const Real & beta)
{
  if (x <= 0.0)
    return 0.0;
  else if (x >= 1.0)
    return 1.0;
  else
    return incompleteBeta(alpha, beta, x);
}

Real
Beta::quantile(const Real & p, const Real & alpha, const Real & beta)
{
  return incompleteBetaInv(alpha, beta, p);
}

Real
Beta::pdf(const Real & x) const
{
  return pdf(x, _alpha, _beta);
}

Real
Beta::cdf(const Real & x) const
{
  return cdf(x, _alpha, _beta);
}

Real
Beta::quantile(const Real & p) const
{
  return quantile(p, _alpha, _beta);
}

Real
Beta::betaFunction(const Real & a, const Real & b)
{
  return std::tgamma(a) * std::tgamma(b) / std::tgamma(a + b);
}

Real
Beta::incompleteBeta(const Real & a, const Real & b, const Real & x)
{
  if (x > ((a + 1.0) / (a + b + 2.0)))
    return 1.0 - incompleteBeta(b, a, 1.0 - x);

  const Real tol = 1e-14;
  const unsigned int max_iter = 1e3;
  const Real coef = std::pow(x, a) * std::pow(1.0 - x, b) / a / betaFunction(a, b);
  Real fn = 1.0;
  Real cn = 1.0;
  Real dn = 0.0;
  Real num;
  Real m = 0.0;
  for (unsigned int i = 0; i < max_iter; ++i)
  {
    if (i == 0)
      num = 1.0;
    else if (i % 2 == 0)
    {
      m = i / 2.0;
      num = m * (b - m) * x / (a + 2.0 * m) / (a + 2.0 * m - 1.0);
    }
    else
    {
      m = (i - 1) / 2.0;
      num = -(a + m) * (a + b + m) * x / (a + 2.0 * m) / (a + 2.0 * m + 1.0);
    }

    dn = 1.0 / (1.0 + num * dn);
    cn = 1.0 + num / cn;
    const Real cd = cn * dn;
    fn *= cd;

    if (std::abs(1.0 - cd) < tol)
      return coef * (fn - 1.0);
  }

  mooseAssert(false, "Could not compute incomplete beta function.");
  return coef * (fn - 1.0);
}

Real
Beta::incompleteBetaInv(const Real & a, const Real & b, const Real & p)
{
  if (a < b)
    return 1.0 - incompleteBetaInv(b, a, 1.0 - p);

  // Try with Newton method
  Real x = 0.5;
  const Real tol = 1e-14;
  const unsigned int max_iter = 1e3;
  const Real scale = betaFunction(a, b);
  Real f;
  Real df;
  for (unsigned int i = 0; i < max_iter; ++i)
  {
    f = incompleteBeta(a, b, x) - p;
    if (std::abs(f) < tol)
      return x;
    df = std::pow(x, a - 1.0) * std::pow(1.0 - x, b - 1.0) / scale;
    x -= f / df;
    // There's a divergence so try out bisection
    if (x < 0 || x > 1.0)
      break;
  }

  // Try with bisection method
  Real x1 = 0; // f(0) = -p
  Real x2 = 1; // f(1) = 1 - p
  for (unsigned int i = 0; i < max_iter; ++i)
  {
    x = (x2 + x1) / 2.0;
    f = incompleteBeta(a, b, x) - p;
    if (std::abs(f) < tol)
      return x;
    else if (f < 0)
      x1 = x;
    else
      x2 = x;
  }

  mooseAssert(false, "Could not find inverse of incomplete gamma function.");
  return x;
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FDistribution.h"
#include "Beta.h"

registerMooseObject("StochasticToolsApp", FDistribution);

InputParameters
FDistribution::validParams()
{
  InputParameters params = Distribution::validParams();
  params.addClassDescription("F-distribution or Fisher-Snedecor distribution");
  params.addRequiredRangeCheckedParam<unsigned int>("df1", "df1 > 0", "Degrees of freedom 1.");
  params.addRequiredRangeCheckedParam<unsigned int>("df2", "df2 > 0", "Degrees of freedom 2.");
  return params;
}

FDistribution::FDistribution(const InputParameters & parameters)
  : Distribution(parameters),
    _df1(getParam<unsigned int>("df1")),
    _df2(getParam<unsigned int>("df2"))
{
}

Real
FDistribution::pdf(const Real & x, const unsigned int & df1, const unsigned int & df2)
{
  // Handy definitions
  Real d1 = static_cast<Real>(df1);
  Real d2 = static_cast<Real>(df2);
  Real d1x = d1 * x;
  Real d2pd1x = d2 + d1x;

  Real a;
  Real b;
  Real y;
  Real z;
  if (d1 * x > d2)
  {
    a = d2 / 2.0;
    b = d1 / 2.0;
    y = (d2 * d1) / (d2pd1x * d2pd1x);
    z = d2 / d2pd1x;
  }
  else
  {
    a = d1 / 2.0;
    b = d2 / 2.0;
    y = (d2pd1x * d1 - d1x * d1) / (d2pd1x * d2pd1x);
    z = d1x / d2pd1x;
  }

  return y * Beta::pdf(z, a, b);
}

Real
FDistribution::cdf(const Real & x, const unsigned int & df1, const unsigned int & df2)
{
  // Handy definitions
  Real d1 = static_cast<Real>(df1);
  Real d2 = static_cast<Real>(df2);

  return Beta::incompleteBeta(d1 / 2.0, d2 / 2.0, d1 * x / (d1 * x + d2));
}

Real
FDistribution::quantile(const Real & p, const unsigned int & df1, const unsigned int & df2)
{
  // Handy definitions
  Real d1 = static_cast<Real>(df1);
  Real d2 = static_cast<Real>(df2);

  Real z = Beta::incompleteBetaInv(d1 / 2.0, d2 / 2.0, p);
  return d2 * z / d1 / (1.0 - z);
}

Real
FDistribution::pdf(const Real & x) const
{
  return pdf(x, _df1, _df2);
}

Real
FDistribution::cdf(const Real & x) const
{
  return cdf(x, _df1, _df2);
}

Real
FDistribution::quantile(const Real & p) const
{
  return quantile(p, _df1, _df2);
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "JohnsonSB.h"
#include "math.h"
#include "libmesh/utility.h"

registerMooseObject("StochasticToolsApp", JohnsonSB);

InputParameters
JohnsonSB::validParams()
{
  InputParameters params = Normal::validParams();
  params.addClassDescription("Johnson Special Bounded (SB) distribution.");

  params.set<Real>("mean") = 0.0;
  params.set<Real>("standard_deviation") = 1.0;
  params.suppressParameter<Real>("mean");
  params.suppressParameter<Real>("standard_deviation");

  params.addRequiredParam<Real>("a", "Lower location parameter");
  params.addRequiredParam<Real>("b", "Upper location parameter");
  params.addRequiredParam<Real>("alpha_1", "Shape parameter (sometimes called a)");
  params.addRequiredParam<Real>("alpha_2", "Shape parameter (sometimes called b)");

  return params;
}

JohnsonSB::JohnsonSB(const InputParameters & parameters)
  : Normal(parameters),
    _lower(getParam<Real>("a")),
    _upper(getParam<Real>("b")),
    _alpha_1(getParam<Real>("alpha_1")),
    _alpha_2(getParam<Real>("alpha_2"))
{
}

Real
JohnsonSB::pdf(
    const Real & x, const Real & a, const Real & b, const Real & alpha_1, const Real & alpha_2)
{
  if (x <= a)
    return 0.0;
  else if (x < b)
  {
    return (alpha_2 * (b - a)) / ((x - a) * (b - x) * std::sqrt(2.0 * M_PI)) *
           std::exp(-0.5 * Utility::pow<2>(alpha_1 + alpha_2 * std::log((x - a) / (b - x))));
  }
  else
    return 0.0;
}

Real
JohnsonSB::cdf(
    const Real & x, const Real & a, const Real & b, const Real & alpha_1, const Real & alpha_2)
{
  if (x <= a)
    return 0.0;
  else if (x < b)
  {
    return Normal::cdf(alpha_1 + alpha_2 * std::log((x - a) / (b - x)), 0.0, 1.0);
  }
  else
    return 0.0;
}

Real
JohnsonSB::quantile(
    const Real & p, const Real & a, const Real & b, const Real & alpha_1, const Real & alpha_2)
{
  const Real Z = Normal::quantile(p, 0.0, 1.0);
  return (a + b * std::exp((Z - alpha_1) / alpha_2)) / (1.0 + std::exp((Z - alpha_1) / alpha_2));
}

Real
JohnsonSB::pdf(const Real & x) const
{
  return pdf(x, _lower, _upper, _alpha_1, _alpha_2);
}

Real
JohnsonSB::cdf(const Real & x) const
{
  return cdf(x, _lower, _upper, _alpha_1, _alpha_2);
}

Real
JohnsonSB::quantile(const Real & p) const
{
  return quantile(p, _lower, _upper, _alpha_1, _alpha_2);
}

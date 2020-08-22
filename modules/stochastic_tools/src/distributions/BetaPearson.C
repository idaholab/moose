//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BetaPearson.h"
#include "math.h"
#include "libmesh/utility.h"

registerMooseObject("StochasticToolsApp", BetaPearson);

InputParameters
BetaPearson::validParams()
{
  InputParameters params = Distribution::validParams();
  params.addClassDescription("BetaPearson distribution");
  params.addRequiredParam<Real>("a", "Parameter a of the Beta distribution.");
  params.addRequiredRangeCheckedParam<Real>(
      "b", "b > 0", "Parameter b of the Beta distribution.");
  params.addRequiredParam<Real>("location", "Location of the Beta distribution.");
  params.addRequiredParam<Real>("scale", "Scale of the Beta distribution.");
  return params;
}

BetaPearson::BetaPearson(const InputParameters & parameters)
  : Distribution(parameters),
    _a(getParam<Real>("a")),
    _b(getParam<Real>("b")),
    _location(getParam<Real>("location")),
    _scale(getParam<Real>("scale"))
{
}

Real
BetaPearson::pdf(const Real & x, const Real & a, const Real & b, const Real & location, const Real & scale)
{
  if (((x - location) / scale) > 1 || ((x - location) / scale) < 0){
    ::mooseError("Error in BetaPearson distribution. Input value outside the bounds.");
  } else {
    const Real betafunc = std::tgamma(a) * std::tgamma(b) / std::tgamma(a + b);
    return std::pow((x - location) / scale, a-1) * std::pow(1-(x - location) / scale, b-1) / betafunc;
  }
}

Real
BetaPearson::cdf(const Real & x, const Real & a, const Real & b, const Real & location, const Real & scale)
{
  // Approximated by summing the pdf since Beta cdf is harder to compute
  if (((x - location) / scale) > 1 || ((x - location) / scale) < 0){
    ::mooseError("Error in BetaPearson distribution. Input value outside the bounds.");
  } else {
    const Real interval = 0.001 * (x - location) / scale;
    const Real betafunc = std::tgamma(a) * std::tgamma(b) / std::tgamma(a + b);
    Real sumcdf = 0;
    for (unsigned i = 0; i < 1000; ++i){
      Real inp = i * interval;
      sumcdf = sumcdf + interval * std::pow(inp, a-1) * std::pow((1-inp), b-1) / betafunc;
    }
    return sumcdf;
  }
}

Real
BetaPearson::quantile(const Real & p, const Real & a, const Real & b, const Real & location, const Real & scale)
{
  // Approximated using the cdf since Beta quantile is harder to compute
  Real value = 0;
  for (unsigned i = 0; i <= 1000; ++i){
    value = i * 0.001;
    Real difference = std::abs((p - BetaPearson::cdf(value, a, b, 0, 1)));
    if (difference < 0.005){
      break;
    }
  }
  return (value * scale + location);
}

Real
BetaPearson::pdf(const Real & x) const
{
  TIME_SECTION(_perf_pdf);
  return pdf(x, _a, _b, _location, _scale);
}

Real
BetaPearson::cdf(const Real & x) const
{
  TIME_SECTION(_perf_cdf);
  return cdf(x, _a, _b, _location, _scale);
}

Real
BetaPearson::quantile(const Real & p) const
{
  TIME_SECTION(_perf_quantile);
  return quantile(p, _a, _b, _location, _scale);
}

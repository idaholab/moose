//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Logistic.h"
#include "math.h"
#include "libmesh/utility.h"

registerMooseObject("StochasticToolsApp", Logistic);

InputParameters
Logistic::validParams()
{
  InputParameters params = Distribution::validParams();
  params.addClassDescription("Logistic distribution.");
  params.addRequiredParam<Real>("location", "Location or mean of the distribution (alpha or mu)");
  params.addRequiredParam<Real>("shape", "Shape of the distribution (beta or s)");
  return params;
}

Logistic::Logistic(const InputParameters & parameters)
  : Distribution(parameters), _location(getParam<Real>("location")), _shape(getParam<Real>("shape"))
{
}

Real
Logistic::pdf(const Real & x, const Real & location, const Real & shape)
{
  Real z = std::exp(-(x - location) / shape);
  return z / (shape * Utility::pow<2>(1.0 + z));
}

Real
Logistic::cdf(const Real & x, const Real & location, const Real & shape)
{
  Real z = std::exp(-(x - location) / shape);
  return 1.0 / (1.0 + z);
}

Real
Logistic::quantile(const Real & p, const Real & location, const Real & shape)
{
  return location - shape * std::log(1.0 / p - 1.0);
}

Real
Logistic::pdf(const Real & x) const
{
  return pdf(x, _location, _shape);
}

Real
Logistic::cdf(const Real & x) const
{
  return cdf(x, _location, _shape);
}

Real
Logistic::quantile(const Real & p) const
{
  return quantile(p, _location, _shape);
}

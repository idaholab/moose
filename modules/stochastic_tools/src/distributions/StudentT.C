//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StudentT.h"
#include "Beta.h"

registerMooseObject("StochasticToolsApp", StudentT);

InputParameters
StudentT::validParams()
{
  InputParameters params = Distribution::validParams();
  params.addClassDescription("Student t-distribution");
  params.addRequiredRangeCheckedParam<unsigned int>("dof", "dof > 0", "Degrees of freedom.");
  return params;
}

StudentT::StudentT(const InputParameters & parameters)
  : Distribution(parameters), _dof(getParam<unsigned int>("dof"))
{
}

Real
StudentT::pdf(const Real & x, const unsigned int & dof)
{
  Real v = dof;
  return std::pow((v / (v + x * x)), (1.0 + v) / 2.0) /
         (std::sqrt(v) * Beta::betaFunction(v / 2.0, 0.5));
}

Real
StudentT::cdf(const Real & x, const unsigned int & dof)
{
  Real v = dof;
  Real z = Beta::incompleteBeta(v / 2.0, 0.5, v / (v + x * x)) / 2.0;
  return x > 0 ? 1.0 - z : z;
}

Real
StudentT::quantile(const Real & p, const unsigned int & dof)
{
  Real v = dof;
  Real x = Beta::incompleteBetaInv(v / 2.0, 0.5, 2.0 * (p <= 0.5 ? p : 1.0 - p));
  return (p <= 0.5 ? -1.0 : 1.0) * std::sqrt(v * (1 - x) / x);
}

Real
StudentT::pdf(const Real & x) const
{
  return pdf(x, _dof);
}

Real
StudentT::cdf(const Real & x) const
{
  return cdf(x, _dof);
}

Real
StudentT::quantile(const Real & p) const
{
  return quantile(p, _dof);
}

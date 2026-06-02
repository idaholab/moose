//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Normal.h"

registerMooseObject("StochasticToolsApp", Normal);

InputParameters
Normal::validParams()
{
  InputParameters params = Distribution::validParams();
  params.addClassDescription("Normal distribution");
  params.addRequiredParam<Real>("mean", "Mean (or expectation) of the distribution.");
  params.addRequiredRangeCheckedParam<Real>(
      "standard_deviation", "standard_deviation > 0", "Standard deviation of the distribution ");
  return params;
}

Normal::Normal(const InputParameters & parameters)
  : Distribution(parameters),
    NormalBase(getParam<Real>("mean"), getParam<Real>("standard_deviation"))
{
}

Real
Normal::pdf(const Real & x) const
{
  return NormalBase::pdf(x, _mean, _standard_deviation);
}

Real
Normal::cdf(const Real & x) const
{
  return NormalBase::cdf(x, _mean, _standard_deviation);
}

Real
Normal::quantile(const Real & p) const
{
  return NormalBase::quantile(p, _mean, _standard_deviation);
}

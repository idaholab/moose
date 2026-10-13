//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KLExponentialCovariance.h"

registerMooseObject("StochasticToolsApp", KLExponentialCovariance);

InputParameters
KLExponentialCovariance::validParams()
{
  InputParameters params = KLCovarianceBase::validParams();
  params.addClassDescription(
      "Exponential covariance kernel: variance * exp(-|x1-x2|/length_scale).");
  params.addRequiredRangeCheckedParam<Real>("variance", "variance > 0", "Marginal variance");
  params.addRequiredRangeCheckedParam<Real>(
      "length_scale", "length_scale > 0", "Correlation length");
  return params;
}

KLExponentialCovariance::KLExponentialCovariance(const InputParameters & parameters)
  : KLCovarianceBase(parameters),
    _variance(getParam<Real>("variance")),
    _length_scale(getParam<Real>("length_scale"))
{
}

Real
KLExponentialCovariance::computeCovariance(Real x1, Real x2) const
{
  return _variance * std::exp(-std::abs(x1 - x2) / _length_scale);
}

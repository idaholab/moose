//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KLSquaredExponentialCovariance.h"

registerMooseObject("StochasticToolsApp", KLSquaredExponentialCovariance);

InputParameters
KLSquaredExponentialCovariance::validParams()
{
  InputParameters params = KLCovarianceBase::validParams();
  params.addClassDescription("Squared-exponential (Gaussian/RBF) covariance kernel: "
                             "variance * exp(-(x1-x2)^2 / (2*length_scale^2)).");
  params.addRequiredRangeCheckedParam<Real>("variance", "variance > 0", "Marginal variance");
  params.addRequiredRangeCheckedParam<Real>(
      "length_scale", "length_scale > 0", "Correlation length");
  return params;
}

KLSquaredExponentialCovariance::KLSquaredExponentialCovariance(const InputParameters & parameters)
  : KLCovarianceBase(parameters),
    _variance(getParam<Real>("variance")),
    _length_scale(getParam<Real>("length_scale"))
{
}

Real
KLSquaredExponentialCovariance::computeCovariance(Real x1, Real x2) const
{
  const Real diff = x1 - x2;
  return _variance * std::exp(-(diff * diff) / (2.0 * _length_scale * _length_scale));
}

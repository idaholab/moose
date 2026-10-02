//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KLCovarianceBase.h"

/**
 * One-dimensional covariance marginal for KLExpansionUserObject:
 * C(x1, x2) = variance * exp( -(x1 - x2)^2 / (2 * length_scale^2) )
 */
class KLSquaredExponentialCovariance : public KLCovarianceBase
{
public:
  static InputParameters validParams();
  KLSquaredExponentialCovariance(const InputParameters & parameters);

  virtual Real computeCovariance(Real x1, Real x2) const override;

protected:
  /// Marginal variance
  const Real _variance;
  /// Correlation length
  const Real _length_scale;
};

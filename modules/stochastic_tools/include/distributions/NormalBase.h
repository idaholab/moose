//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libMeshReducedNamespace.h"
/**
 * Helper Class that contains all the logic required for sampling from normal distributions
 *
 */
class NormalBase
{
public:
  NormalBase(const Real mean, const Real standard_deviation);

  static Real pdf(const Real & x, const Real & mean, const Real & std_dev);
  static Real cdf(const Real & x, const Real & mean, const Real & std_dev);
  static Real quantile(const Real & p, const Real & mean, const Real & std_dev);

  Real pdf(const Real & x) const;
  Real cdf(const Real & x) const;
  Real quantile(const Real & p) const;

protected:
  ///@{
  /// Coefficients for the rational function used to approximate the quantile
  static const std::array<Real, 6> _a;
  static const std::array<Real, 6> _b;
  ///@}
  /// The mean (or expectation) of the distribution (mu)
  const Real _mean;

  /// The standard deviation of the distribution (sigma)
  const Real _standard_deviation;
};

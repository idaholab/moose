//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Distribution.h"

class NormalDistribution;

template <>
InputParameters validParams<NormalDistribution>();

/**
 * A class used to generate a normal distribution
 */
class NormalDistribution : public Distribution
{
public:
  NormalDistribution(const InputParameters & parameters);

  virtual Real pdf(const Real & x) const override;
  virtual Real cdf(const Real & x) const override;
  virtual Real quantile(const Real & p) const override;

  Real pdf(const Real & x, const Real & mean, const Real & std_dev) const;
  Real cdf(const Real & x, const Real & mean, const Real & std_dev) const;
  Real quantile(const Real & p, const Real & mean, const Real & std_dev) const;

protected:
  ///@{
  /// Coefficients for the rational function used to approximate the quantile
  const std::vector<Real> _a = {
      -0.322232431088, -1.0, -0.342242088547, -0.0204231210245, -0.0000453642210148};

  const std::vector<Real> _b = {
      0.099348462606, 0.588581570495, 0.531103462366, 0.10353775285, 0.0038560700634};
  ///@}

  /// The mean (or expectation) of the distribution (mu)
  const Real & _mean;

  /// The standard deviation of the distribution (sigma)
  const Real & _standard_deviation;
};


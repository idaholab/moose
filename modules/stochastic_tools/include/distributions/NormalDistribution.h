//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NORMALDISTRIBUTION_H
#define NORMALDISTRIBUTION_H

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

  virtual Real pdf(const Real & x) override;
  virtual Real cdf(const Real & x) override;

  /**
   * Compute the quantile
   * @param p The cdf for which the quantile is evaluated
   **/
  virtual Real quantile(const Real & p) override;

protected:
  /// Coefficients for the rational function used to approximate the quantile:
  const std::vector<Real> _a;
  const std::vector<Real> _b;

  /// The mean (or expectation) of the distribution (mu)
  Real _mean;

  /// The standard deviation of the distribution (sigma)
  Real _standard_deviation;
};

#endif /* NORMALDISTRIBUTION_H */

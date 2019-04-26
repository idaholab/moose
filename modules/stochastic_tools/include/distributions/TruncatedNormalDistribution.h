//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NormalDistribution.h"

class TruncatedNormalDistribution;

template <>
InputParameters validParams<TruncatedNormalDistribution>();

/**
 * A class used to generate a truncated normal distribution
 */
class TruncatedNormalDistribution : public NormalDistribution
{
public:
  TruncatedNormalDistribution(const InputParameters & parameters);

  virtual Real pdf(const Real & x) const override;
  virtual Real cdf(const Real & x) const override;
  virtual Real quantile(const Real & p) const override;

  Real pdf(const Real & x,
           const Real & mean,
           const Real & std_dev,
           const Real & lower_bound,
           const Real & upper_bound) const;
  Real cdf(const Real & x,
           const Real & mean,
           const Real & std_dev,
           const Real & lower_bound,
           const Real & upper_bound) const;
  Real quantile(const Real & p,
                const Real & mean,
                const Real & std_dev,
                const Real & lower_bound,
                const Real & upper_bound) const;

protected:
  /// The lower bound for the distribution
  const Real & _lower_bound;

  /// The upper bound for the distribution
  const Real & _upper_bound;
};


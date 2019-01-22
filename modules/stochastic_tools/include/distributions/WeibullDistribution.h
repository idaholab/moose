//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef WEIBULLDISTRIBUTION_H
#define WEIBULLDISTRIBUTION_H

#include "Distribution.h"

class WeibullDistribution;

template <>
InputParameters validParams<WeibullDistribution>();

/**
 * A class used to generate a three-parameter Weibull distribution
 */
class WeibullDistribution : public Distribution
{
public:
  WeibullDistribution(const InputParameters & parameters);

  virtual Real pdf(const Real & x) override;
  virtual Real cdf(const Real & x) override;
  virtual Real quantile(const Real & p) override;

protected:
  /// The location parameter (a or low)
  const Real & _a;

  /// The scale parameter (b or lambda)
  const Real & _b;

  /// The shape parameter (c or k)
  const Real & _c;
};

#endif /* WEIBULLDISTRIBUTION_H */

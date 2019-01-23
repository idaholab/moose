//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef JOHNSONSBDISTRIBUTION_H
#define JOHNSONSBDISTRIBUTION_H

#include "NormalDistribution.h"

class JohnsonSBDistribution;

template <>
InputParameters validParams<JohnsonSBDistribution>();

/**
 * A class used to generate a Johnson SB distribution
 * This provides a method to set the parameters after it has been constructed.
 */
class JohnsonSBDistribution : public NormalDistribution
{
public:
  JohnsonSBDistribution(const InputParameters & parameters);

  virtual Real pdf(const Real & x) const override;
  virtual Real cdf(const Real & x) const override;
  virtual Real quantile(const Real & p) const override;

  Real pdf(const Real & x, const Real & a, const Real & b, const Real & alpha_1, const Real & alpha_2) const;
  Real cdf(const Real & x, const Real & a, const Real & b, const Real & alpha_1, const Real & alpha_2) const;
  Real quantile(const Real & p, const Real & a, const Real & b, const Real & alpha_1, const Real & alpha_2) const;

protected:
  /// The lower location parameter, a
  const Real & _a;

  /// The upper location parameter, b
  const Real & _b;

  /// The first shape parameter, alpha_1
  const Real & _alpha_1;

  /// The second shape parameter, alpha_2
  const Real & _alpha_2;
};

#endif

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

class LogisticDistribution;

template <>
InputParameters validParams<LogisticDistribution>();

/**
 * A class used to generate a logistic distribution
 */
class LogisticDistribution : public Distribution
{
public:
  LogisticDistribution(const InputParameters & parameters);

  virtual Real pdf(const Real & x) const override;
  virtual Real cdf(const Real & x) const override;
  virtual Real quantile(const Real & p) const override;

  Real pdf(const Real & x, const Real & location, const Real & shape) const;
  Real cdf(const Real & x, const Real & location, const Real & shape) const;
  Real quantile(const Real & p, const Real & location, const Real & shape) const;

protected:
  /// The location or mean of the distribution (alpha or mu)
  const Real & _location;

  /// The shape of the distribution (beta or s)
  const Real & _shape;
};


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

/**
 * A class used to generate uniform distribution
 */
class Uniform : public Distribution
{
public:
  static InputParameters validParams();

  Uniform(const InputParameters & parameters);

  static Real pdf(const Real & x, const Real & lower_bound, const Real & upper_bound);
  static Real cdf(const Real & x, const Real & lower_bound, const Real & upper_bound);
  static Real quantile(const Real & y, const Real & lower_bound, const Real & upper_bound);

  virtual Real pdf(const Real & x) const override;
  virtual Real cdf(const Real & x) const override;
  virtual Real quantile(const Real & y) const override;

protected:
  /// The lower bound for the uniform distribution
  const Real & _lower_bound;

  /// The upper bound for the uniform distribution
  const Real & _upper_bound;
};

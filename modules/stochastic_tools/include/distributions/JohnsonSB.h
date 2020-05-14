//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Normal.h"

/**
 * A class used to generate a Johnson SB distribution
 */
class JohnsonSB : public Normal
{
public:
  static InputParameters validParams();

  JohnsonSB(const InputParameters & parameters);

  virtual Real pdf(const Real & x) const override;
  virtual Real cdf(const Real & x) const override;
  virtual Real quantile(const Real & p) const override;

  static Real
  pdf(const Real & x, const Real & a, const Real & b, const Real & alpha_1, const Real & alpha_2);
  static Real
  cdf(const Real & x, const Real & a, const Real & b, const Real & alpha_1, const Real & alpha_2);
  static Real quantile(
      const Real & p, const Real & a, const Real & b, const Real & alpha_1, const Real & alpha_2);

protected:
  /// The lower location parameter, a
  const Real & _lower;

  /// The upper location parameter, b
  const Real & _upper;

  /// The first shape parameter, alpha_1
  const Real & _alpha_1;

  /// The second shape parameter, alpha_2
  const Real & _alpha_2;
};

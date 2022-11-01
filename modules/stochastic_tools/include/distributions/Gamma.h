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
 * A class used to generate a Gamma distribution
 */
class Gamma : public Distribution
{
public:
  static InputParameters validParams();

  Gamma(const InputParameters & parameters);

  virtual Real pdf(const Real & x) const override;
  virtual Real cdf(const Real & x) const override;
  virtual Real quantile(const Real & p) const override;

  static Real pdf(const Real & x, const Real & alpha, const Real & beta);
  static Real cdf(const Real & x, const Real & alpha, const Real & beta);
  static Real quantile(const Real & p, const Real & alpha, const Real & beta);

  /**
   * Lower incomplete gamma function.
   * Non-boost implementation from:
   * Temme, N. (1994). A Set of Algorithms for the Incomplete Gamma Functions.
   * Probability in the Engineering and Informational Sciences, 8(2), 291-307.
   */
  static Real incompleteGamma(const Real & a, const Real & x);
  /**
   * Inverse of lower incomplete gamma function.
   * Non-boost implementation uses Newton-Raphson to find root of incompleteGamma(a, x) - p
   */
  static Real incompleteGammaInv(const Real & a, const Real & p);

protected:
  /// Shape
  const Real & _alpha;
  /// Scaling
  const Real & _theta;
};

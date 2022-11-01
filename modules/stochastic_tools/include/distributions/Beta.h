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
 * A class used to generate a Beta distribution
 */
class Beta : public Distribution
{
public:
  static InputParameters validParams();

  Beta(const InputParameters & parameters);

  virtual Real pdf(const Real & x) const override;
  virtual Real cdf(const Real & x) const override;
  virtual Real quantile(const Real & p) const override;

  static Real pdf(const Real & x, const Real & alpha, const Real & beta);
  static Real cdf(const Real & x, const Real & alpha, const Real & beta);
  static Real quantile(const Real & p, const Real & alpha, const Real & beta);

  /// Beta function: B(a,b) = Gamma(a)Gamma(b)/Gamma(a+b)
  static Real betaFunction(const Real & a, const Real & b);
  /**
   * Lower incomplete beta function.
   * Non-boost implementation from:
   * Section 6.4 of https://www.cec.uchile.cl/cinetica/pcordero/MC_libros/NumericalRecipesinC.pdf
   */
  static Real incompleteBeta(const Real & a, const Real & b, const Real & x);
  /**
   * Inverse of lower incomplete beta function.
   * Non-boost implementation uses Newton-Raphson to find root of incompleteBeta(a, b, x) - p
   */
  static Real incompleteBetaInv(const Real & a, const Real & b, const Real & p);

protected:
  /// Shape parameter 1
  const Real & _alpha;
  /// Shape parameter 2
  const Real & _beta;
};

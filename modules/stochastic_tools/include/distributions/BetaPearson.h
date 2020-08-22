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
 * A class used to generate a BetaPearson distribution
 */
class BetaPearson : public Distribution
{
public:
  static InputParameters validParams();

  BetaPearson(const InputParameters & parameters);

  virtual Real pdf(const Real & x) const override;
  virtual Real cdf(const Real & x) const override;
  virtual Real quantile(const Real & p) const override;

  static Real pdf(const Real & x, const Real & a, const Real & b, const Real & location, const Real & scale);
  static Real cdf(const Real & x, const Real & a, const Real & b, const Real & location, const Real & scale);
  static Real quantile(const Real & p, const Real & a, const Real & b, const Real & location, const Real & scale);

protected:
  ///@{
  /// Coefficients for the rational function used to approximate the quantile
  // static const std::array<Real, 6> _a;
  // static const std::array<Real, 6> _b;
  ///@}

  /// Parameter a of the Beta distribution
  const Real & _a;

  /// Parameter b of the Beta distribution
  const Real & _b;

  /// Location of the Beta distribution
  const Real & _location;

  /// Scale of the Beta distribution
  const Real & _scale;
};

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LeastSquaresFitBase.h"

/**
 * Least squares polynomial fit
 *
 * Requires: LAPACK
 */
class PolynomialFit : public LeastSquaresFitBase
{
public:
  /* Constructor, Takes two vectors of points for which to apply the fit.  One should be of the
   * independent variable while the other should be of the dependent variable.  These values should
   * correspond to one and other in the same position.  The third parameter is the requested
   * polynomial order and the forth parameter tells the class whether or not it should truncate
   * the order if there are not enough points for which to apply the polynomial fit.
   */
  PolynomialFit(const std::vector<Real> & x,
                const std::vector<Real> & y,
                unsigned int order,
                bool truncate_order = false);

  virtual Real sample(Real x) override;

protected:
  virtual void fillMatrix() override;
  /// Order of the polynomial
  unsigned int _order;
  /// Flag to implement a truncated polynomial
  bool _truncate_order;
  /// File number
  static int _file_number;
};

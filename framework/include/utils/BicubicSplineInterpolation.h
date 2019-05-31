//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SplineInterpolationBase.h"

/**
 * This class interpolates tabulated functions with a bi-cubic spline
 *
 * Adopted from Numerical Recipes in C (section 3.6).
 * Consistent with the terminology in Numerical Recipes, moving over a column spline means moving
 * over the x1 coord
 * Likewise, moving over a row spline means moving over the x2 coord
 */
class BicubicSplineInterpolation : public SplineInterpolationBase
{
public:
  BicubicSplineInterpolation();
  /**
   * In the future, may add interface that allows necessary vector of boundary conditions to be
   * supplied for each edge of grid;
   * however, for now we just use natural splines at the grid boundaries
   */
  BicubicSplineInterpolation(const std::vector<Real> & x1,
                             const std::vector<Real> & x2,
                             const std::vector<std::vector<Real>> & y,
                             const std::vector<Real> & yx11 = std::vector<Real>(),
                             const std::vector<Real> & yx1n = std::vector<Real>(),
                             const std::vector<Real> & yx21 = std::vector<Real>(),
                             const std::vector<Real> & yx2n = std::vector<Real>());

  virtual ~BicubicSplineInterpolation() = default;

  /**
   * Set the x1, x2 and y values, and first derivatives at the edges
   */
  void setData(const std::vector<Real> & x1,
               const std::vector<Real> & x2,
               const std::vector<std::vector<Real>> & y,
               const std::vector<Real> & yx11 = std::vector<Real>(),
               const std::vector<Real> & yx1n = std::vector<Real>(),
               const std::vector<Real> & yx21 = std::vector<Real>(),
               const std::vector<Real> & yx2n = std::vector<Real>());

  /**
   * Sanity checks on input data
   */
  void errorCheck();

  /**
   * Samples value at point (x1, x2)
   */
  Real sample(Real x1, Real x2, Real yx11 = _deriv_bound, Real yx1n = _deriv_bound);

  /**
   * Samples value and first derivatives at point (x1, x2)
   * Use this function for speed when computing both value and derivatives,
   * as it minimizes the amount of spline evaluation
   */
  void sampleValueAndDerivatives(Real x1,
                                 Real x2,
                                 Real & y,
                                 Real & dy1,
                                 Real & dy2,
                                 Real yx11 = _deriv_bound,
                                 Real yx1n = _deriv_bound,
                                 Real yx21 = _deriv_bound,
                                 Real yx2n = _deriv_bound);

  /**
   * Samples first derivative at point (x1, x2)
   */
  Real sampleDerivative(
      Real x1, Real x2, unsigned int deriv_var, Real yp1 = _deriv_bound, Real ypn = _deriv_bound);

  /**
   * Samples second derivative at point (x1, x2)
   */
  Real sample2ndDerivative(
      Real x1, Real x2, unsigned int deriv_var, Real yp1 = _deriv_bound, Real ypn = _deriv_bound);

protected:
  /// Independent values in the x1 direction
  std::vector<Real> _x1;
  /// Independent values in the x2 direction
  std::vector<Real> _x2;
  /// The dependent values at (x1, x2) points
  std::vector<std::vector<Real>> _y;
  /// Transpose of _y
  std::vector<std::vector<Real>> _y_trans;

  /**
   * Boundary conditions. The first index indicates the coordinate
   * the derivative is with respect to. The second index indicates
   * the grid index, e.g. 1 is the lower bound, and n is the upper
   * bound
   */
  std::vector<Real> _yx11;
  std::vector<Real> _yx1n;
  std::vector<Real> _yx21;
  std::vector<Real> _yx2n;

  /// Second derivatives
  std::vector<std::vector<Real>> _y2_rows;
  std::vector<std::vector<Real>> _y2_columns;

  /// Vectors used during sampling
  std::vector<Real> _row_spline_second_derivs;
  std::vector<Real> _row_spline_eval;
  std::vector<Real> _column_spline_second_derivs;
  std::vector<Real> _column_spline_eval;

  /**
   * Precompute tables of row (column) spline second derivatives
   * and store them to reduce computational demands
   */
  void constructRowSplineSecondDerivativeTable();
  void constructColumnSplineSecondDerivativeTable();

  /**
   * Calculates the tables of second derivatives
   */
  void solve();

  /**
   * Helper functions to evaluate column splines and construct row spline
   * for the given point
   */
  void constructRowSpline(Real x1,
                          std::vector<Real> & spline_eval,
                          std::vector<Real> & spline_second_derivs,
                          Real yx11 = _deriv_bound,
                          Real yx1n = _deriv_bound);

  /**
   * Helper functions to evaluate row splines and construct column spline
   * for the given point
   */
  void constructColumnSpline(Real x2,
                             std::vector<Real> & spline_eval,
                             std::vector<Real> & spline_second_derivs,
                             Real yx21 = _deriv_bound,
                             Real yx2n = _deriv_bound);

  /// File number for data dump
  static int _file_number;
};

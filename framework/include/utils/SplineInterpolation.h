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
#include <string>

/**
 * This class interpolates tabulated functions with cubic splines
 *
 * Adopted from Numerical Recipes in C (section 3.3).
 */
class SplineInterpolation : public SplineInterpolationBase
{
public:
  SplineInterpolation();
  /**
   * Construct the object
   * @param x Tabulated function (x-positions)
   * @param y Tabulated function (y-positions)
   * @param yp1 First derivative of the interpolating function at point 1
   * @param ypn First derivative of the interpolating function at point n
   *
   * If yp1, ypn are not specified or greater or equal that _deriv_bound, we use natural spline
   */
  SplineInterpolation(const std::vector<Real> & x,
                      const std::vector<Real> & y,
                      Real yp1 = _deriv_bound,
                      Real ypn = _deriv_bound);

  virtual ~SplineInterpolation() = default;

  /**
   * Set the x-, y- values and first derivatives
   */
  void setData(const std::vector<Real> & x,
               const std::vector<Real> & y,
               Real yp1 = _deriv_bound,
               Real ypn = _deriv_bound);

  void errorCheck();

  /**
   * This function will take an independent variable input and will return the dependent variable
   * based on the generated fit
   */
  Real sample(Real x) const;
  ADReal sample(const ADReal & x) const;

  Real sampleDerivative(Real x) const;

  Real sample2ndDerivative(Real x) const;

  /**
   * This function returns the size of the array holding the points, i.e. the number of sample
   * points
   */
  unsigned int getSampleSize();

  Real domain(int i) const;
  Real range(int i) const;

protected:
  std::vector<Real> _x;
  std::vector<Real> _y;
  /// boundary conditions
  Real _yp1, _ypn;
  /// Second derivatives
  std::vector<Real> _y2;

  void solve();

  static int _file_number;
};

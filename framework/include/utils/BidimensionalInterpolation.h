//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "ColumnMajorMatrix.h"
#include "MooseTypes.h"

// C++ includes
#include <vector>

/**
 * This class interpolates tabulated data with a Bidimension function (either bicubic or bilinear).
 * In order to minimize the computational expense of each sample, the coefficients at each point in
 * the tabulated data are computed once in advance, and then accessed during the interpolation.
 *
 * Adapted from Numerical Recipes in C (section 3.6). The terminology used is
 * consistent with that used in Numerical Recipes, where moving over a column
 * corresponds to moving over the x1 coord. Likewise, moving over a row means
 * moving over the x2 coord.
 */
class BidimensionalInterpolation
{
public:
  BidimensionalInterpolation(const std::vector<Real> & x1, const std::vector<Real> & x2);

  virtual ~BidimensionalInterpolation() = default;

  /// Independent values in the x1 direction
  std::vector<Real> _x1;
  /// Independent values in the x2 direction
  std::vector<Real> _x2;

  /**
   * Samples value at point (x1, x2)
   */
  virtual Real sample(const Real x1, const Real x2) const = 0;
  virtual ADReal sample(const ADReal & x1, const ADReal & x2) const = 0;

  /**
   * Samples first derivative at point (x1, x2)
   */
  virtual Real sampleDerivative(Real /*x1*/, Real /*x2*/, unsigned int /*deriv_var*/) const
  {
    mooseError("sampleDerivative is not implemented for this interpolation class");
  }

  /**
   * Samples second derivative at point (x1, x2)
   */
  virtual Real sample2ndDerivative(Real /*x1*/, Real /*x2*/, unsigned int /*deriv_var*/) const
  {
    mooseError("sample2ndDerivative is not implemented for this interpolation class");
  }

  /**
   * Samples value and first derivatives at point (x1, x2)
   * Use this function for speed when computing both value and derivatives,
   * as it minimizes the amount of time spent locating the point in the
   * tabulated data
   */
  virtual void sampleValueAndDerivatives(
      Real /*x1*/, Real /*x2*/, Real & /* y*/, Real & /* dy1*/, Real & /* dy2*/) const
  {
    mooseError("sampleValueAndDerivatives is not implemented for this interpolation class");
  }

  /**
   * Same as sampleValueAndDerivatives, but ADReal instead of Real
   */
  virtual void sampleValueAndDerivatives(const ADReal & /*x1*/,
                                         const ADReal & /*x2*/,
                                         ADReal & /* y*/,
                                         ADReal & /* dy1*/,
                                         ADReal & /* dy2*/) const
  {
    mooseError("sampleValueAndDerivatives is not implemented for this interpolation class");
  }
};

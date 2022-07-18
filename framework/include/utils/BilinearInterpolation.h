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
#include "BidimensionalInterpolation.h"

// C++ includes
#include <vector>

class BidimensionalInterpolation;

/**
 * This class applies the Least Squares algorithm to a set of points
 * to provide a smooth curve for sampling values.
 * BilinearInterpolation is designed to linearly interpolate a
 * function of two values e.g. z(x,y).  Supply Bilinearlinear with a
 * vector of x and a vector of y and a ColumnMajorMatrix of function
 * values, z, that correspond to the values in the vectors x and
 * y...and also a sample point (x1 and x2), and
 * BilinearInterpolation will return the value of the function at the
 * sample point.  A simple example:
 *
 * x = [1 2], y = [1 2],
 *
 * z = [1 2]
 *     [3 4]
 *
 * with x1 = 1.5 and x2 = 1.5 returns a value of 2.5.
 */
class BilinearInterpolation : public BidimensionalInterpolation
{
public:
  /**
   *  Constructor, Takes two vectors of points for which to apply the
   * fit.  One should be of the independent variable while the other
   * should be of the dependent variable.  These values should
   * correspond to one and other in the same position.
   */
  BilinearInterpolation(const std::vector<Real> & XAXIS,
                        const std::vector<Real> & YAXIS,
                        const ColumnMajorMatrix & ZSURFACE);

  virtual ~BilinearInterpolation() = default;

  /**
   * This function will take an independent variable input and will
   * return the dependent variable based on the generated fit.
   */
  Real sample(const Real x1, const Real x2) const override;
  ADReal sample(const ADReal & x1, const ADReal & x2) const override;

  /**
   * Samples value and first derivatives at point (x1, x2)
   * Use this function for speed when computing both value and derivatives,
   * as it minimizes the amount of time spent locating the point in the
   * tabulated data
   */
  Real sampleDerivative(Real x1, Real x2, unsigned int deriv_var) const override;

  void sampleValueAndDerivatives(Real x1, Real x2, Real & y, Real & dy1, Real & dy2) const override;

  void
  getNeighborIndices(const std::vector<Real> & inArr, Real x, int & lowerX, int & upperX) const;

private:
  /// sampleInternal only used by BilinearInterpolation, hence made private
  template <typename T>
  T sampleInternal(T & x1, T & x2) const;

  ColumnMajorMatrix _zSurface;
  static int _file_number;
};

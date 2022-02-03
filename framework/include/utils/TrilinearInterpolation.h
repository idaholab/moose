//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <vector>
#include "Moose.h"

/**
 * This class interpolates a function of three values (f(x,y,z)).  It takes 4
 * vectors for x, y, z, and function values, f(x,y,z).  The vector of function
 * values should be done in the following manner:
 * Function values of constant x and y are written, corresponding with values
 * in vector, z.
 * Function values for the constant x, next y-value, corresponding with values
 * in vector, z.
 * After last y-value, function values for the next x, first y-value,
 * corresponding with values in vector, z.
 *
 * An example:
 * f(1,4,7) = 10, f(1,4,8) = 11, f(1,4,9) = 12
 * f(1,5,7) = 13, f(1,5,8) = 14, f(1,5,9) = 15
 * f(1,6,7) = 16, f(1,6,8) = 17, f(1,6,9) = 18
 * f(2,4,7) = 20, f(2,4,8) = 21, f(2,4,9) = 22
 * f(2,5,7) = 23, f(2,5,8) = 24, f(2,5,9) = 25
 * f(2,6,7) = 26, f(2,6,8) = 27, f(2,6,9) = 28
 * f(3,4,7) = 30, f(3,4,8) = 31, f(3,4,9) = 32
 * f(3,5,7) = 33, f(3,5,8) = 34, f(3,5,9) = 35
 * f(3,6,7) = 36, f(3,6,8) = 37, f(3,6,9) = 38
 *
 * x = {1, 2, 3};
 * y = {4, 5, 6};
 * z = {7, 8, 9};
 * fxyz = {
 *  // fxyz for x = 1
 *  10, 11, 12,
 *  13, 14, 15,
 *  16, 17, 18,
 *  // fxyz for x = 2
 *  20, 21, 22,
 *  23, 24, 25,
 *  26, 27, 28,
 *  // fxyz for x = 3
 *  30, 31, 32,
 *  33, 34, 35,
 *  36, 37, 38
 *  };
 *
 */
class TrilinearInterpolation
{
public:
  /**
   * Constructor initializes data for interpolation
   * @param x vector for x-coordinates
   * @param y vector for y-coordinates
   * @param z vector for z-coordinates
   * @param data vector for function values formatted in the same manner as the example
   */
  TrilinearInterpolation(const std::vector<Real> & x,
                         const std::vector<Real> & y,
                         const std::vector<Real> & z,
                         const std::vector<Real> & data);

  virtual ~TrilinearInterpolation() = default;

  /**
   * Interpolates for the desired (x,y,z) coordinate and returns the value
   * based on the function values vector.
   * @param x desired x-coordinate
   * @param y desired y-coordinate
   * @param z desired z-coordinate
   * @return interpolated value at coordinate (x,y,z)
   */
  Real sample(Real x, Real y, Real z) const;

protected:
  /// vector of x-values
  std::vector<Real> _x_axis;

  /// vector of y-values
  std::vector<Real> _y_axis;

  /// vector of z-values
  std::vector<Real> _z_axis;

  /// vector of function values, f(x,y,z)
  std::vector<Real> _fxyz;

  /**
   * Finds the indices of the cube that point (x,y,z) is in.
   * @param[in] v vector to find lower and upper limits for the cube
   * @param[in] x desired coordinate
   * @param[out] lower lower limit for cube
   * @param[out] upper upper limit for cube
   * @param[out] d ratio of (x - lower) / (upper - lower)
   */
  void
  getCornerIndices(const std::vector<Real> & v, Real x, int & lower, int & upper, Real & d) const;

  /**
   * Searches the function value vector for the value at a given corner
   * coordinate from the getCornerIndices function.
   * @param x index for x-coordinate of corner
   * @param y index for y-coordinate of corner
   * @param z index for z-coordinate of corner
   * @return function value for the (x,y,z) coordinate
   */
  Real getCornerValues(int x, int y, int z) const;
};

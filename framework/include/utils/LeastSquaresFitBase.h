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
#include <string>

#include "Moose.h"

/**
 * Base class for linear least squares fit method. Derived classes must implement
 * the fillMatrix() and sample() methods with the functional form of the fit.
 *
 * Requires: LAPACK
 */
class LeastSquaresFitBase
{
public:
  LeastSquaresFitBase();
  LeastSquaresFitBase(const std::vector<Real> & x, const std::vector<Real> & y);

  virtual ~LeastSquaresFitBase() = default;

  /**
   * Generate the fit. This function must be called prior to using sample.
   * Note: If you pass a vector that contains duplicate independent measures
   * the call to LAPACK will fail
   */
  virtual void generate();

  /**
   * This function will take an independent variable input and will return the
   * dependent variable based on the generated fit
   * @param x independent variable
   * @return dependent variable
   */
  virtual Real sample(Real x) = 0;

  /**
   * Size of the array holding the points
   * @return number of sample points
   */
  unsigned int getSampleSize();

  /**
   * Const reference to the vector of coefficients of the least squares fit
   * @param return vector of coefficients
   */
  const std::vector<Real> & getCoefficients();

  void setVariables(const std::vector<Real> & x, const std::vector<Real> & y);

protected:
  /**
   * Helper function that creates the matrix necessary for the least squares algorithm
   */
  virtual void fillMatrix() = 0;

  /**
   * Wrapper for the LAPACK dgels function. Called by generate() to perform the
   * least squares fit
   */
  void doLeastSquares();

  /// Independent variable
  std::vector<Real> _x;
  /// Dependent variable
  std::vector<Real> _y;
  /// Basis functions evaluated at each independent variable (note: actually a vector)
  std::vector<Real> _matrix;
  /// Vector of coefficients of the least squares fit
  std::vector<Real> _coeffs;
  /// The number of coefficients
  unsigned int _num_coeff;
};

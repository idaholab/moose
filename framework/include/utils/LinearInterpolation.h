//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Moose.h"
#include "MooseTypes.h"

#include <vector>
#include <string>

/**
 * This class interpolates values given a set of data pairs and an abscissa.
 */
class LinearInterpolation
{
public:
  /* Constructor, Takes two vectors of points for which to apply the fit.  One should be of the
   * independent variable while the other should be of the dependent variable.  These values should
   * correspond to one and other in the same position.
   */
  LinearInterpolation(const std::vector<Real> & X,
                      const std::vector<Real> & Y,
                      const bool extrap = false);
  LinearInterpolation() : _x(std::vector<Real>()), _y(std::vector<Real>()), _extrap(false) {}

  virtual ~LinearInterpolation() = default;

  /**
   * Set the x and y values.
   */
  void setData(const std::vector<Real> & X, const std::vector<Real> & Y)
  {
    _x = X;
    _y = Y;
    errorCheck();
  }

  void errorCheck();

  /**
   * This function will take an independent variable input and will return the dependent variable
   * based on the generated fit
   */
  template <typename T>
  T sample(const T & x) const;

  /**
   * This function will take an independent variable input and will return the derivative of the
   * dependent variable
   * with respect to the independent variable based on the generated fit
   */
  template <typename T>
  T sampleDerivative(const T & x) const;

  /**
   * This function returns the size of the array holding the points, i.e. the number of sample
   * points
   */
  unsigned int getSampleSize() const;

  /**
   * This function returns the integral of the function
   */
  Real integrate();

  Real domain(int i) const;
  Real range(int i) const;

private:
  std::vector<Real> _x;
  std::vector<Real> _y;

  bool _extrap;
};

// for backwards compatibility
typedef LinearInterpolation ADLinearInterpolation;

// temporary fixes to avoid breaking bison
template <typename T>
class LinearInterpolationTempl : public LinearInterpolation
{
public:
  using LinearInterpolation::LinearInterpolation;
};

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include <vector>

class SplineInterpolationBase
{
public:
  SplineInterpolationBase();

  virtual ~SplineInterpolationBase() = default;

  Real sample(const std::vector<Real> & x,
              const std::vector<Real> & y,
              const std::vector<Real> & y2,
              Real x_int) const;
  ADReal sample(const std::vector<Real> & x,
                const std::vector<Real> & y,
                const std::vector<Real> & y2,
                const ADReal & x_int) const;

  Real sampleDerivative(const std::vector<Real> & x,
                        const std::vector<Real> & y,
                        const std::vector<Real> & y2,
                        Real x_int) const;

  Real sample2ndDerivative(const std::vector<Real> & x,
                           const std::vector<Real> & y,
                           const std::vector<Real> & y2,
                           Real x_int) const;

protected:
  /**
   * This function calculates the second derivatives based on supplied x and y-vectors
   */
  void spline(const std::vector<Real> & x,
              const std::vector<Real> & y,
              std::vector<Real> & y2,
              Real yp1 = _deriv_bound,
              Real ypn = _deriv_bound);

  void findInterval(const std::vector<Real> & x,
                    Real x_int,
                    unsigned int & klo,
                    unsigned int & khi) const;

  template <typename T>
  void computeCoeffs(const std::vector<Real> & x,
                     unsigned int klo,
                     unsigned int khi,
                     const T & x_int,
                     Real & h,
                     T & a,
                     T & b) const;

  /**
   * Sample value at point x_int given the indices of the vector of
   * dependent values that bound the point. This method is useful
   * in bicubic spline interpolation, where several spline evaluations
   * are needed to sample from a 2D point.
   */
  template <typename T>
  T sample(const std::vector<Real> & x,
           const std::vector<Real> & y,
           const std::vector<Real> & y2,
           const T & x_int,
           unsigned int klo,
           unsigned int khi) const;

  static const Real _deriv_bound;
};

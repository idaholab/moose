//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SPLINEINTERPOLATIONBASE_H
#define SPLINEINTERPOLATIONBASE_H

#include <vector>
#include "libmesh/libmesh_common.h"
using libMesh::Real;

class SplineInterpolationBase
{
public:
  SplineInterpolationBase();

  virtual ~SplineInterpolationBase() = default;

  Real sample(const std::vector<Real> & x,
              const std::vector<Real> & y,
              const std::vector<Real> & y2,
              Real x_int) const;
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
  void computeCoeffs(const std::vector<Real> & x,
                     unsigned int klo,
                     unsigned int khi,
                     Real x_int,
                     Real & h,
                     Real & a,
                     Real & b) const;

  static const Real _deriv_bound;
};

#endif

/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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

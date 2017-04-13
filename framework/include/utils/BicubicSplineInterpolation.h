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

#ifndef BICUBICSPLINEINTERPOLATION_H
#define BICUBICSPLINEINTERPOLATION_H

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
   * Set the x1-, x2, y- values and first derivatives
   */
  void setData(const std::vector<Real> & x1,
               const std::vector<Real> & x2,
               const std::vector<std::vector<Real>> & y,
               const std::vector<Real> & yx11 = std::vector<Real>(),
               const std::vector<Real> & yx1n = std::vector<Real>(),
               const std::vector<Real> & yx21 = std::vector<Real>(),
               const std::vector<Real> & yx2n = std::vector<Real>());

  void errorCheck();

  Real sample(Real x1, Real x2, Real yx11 = _deriv_bound, Real yx1n = _deriv_bound);
  Real sampleDerivative(
      Real x1, Real x2, unsigned int deriv_var, Real yp1 = _deriv_bound, Real ypn = _deriv_bound);
  Real sample2ndDerivative(
      Real x1, Real x2, unsigned int deriv_var, Real yp1 = _deriv_bound, Real ypn = _deriv_bound);

protected:
  std::vector<Real> _x1;
  std::vector<Real> _x2;
  std::vector<std::vector<Real>> _y;

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

  void constructRowSplineSecondDerivativeTable();
  void constructColumnSplineSecondDerivativeTable();
  void solve();

  static int _file_number;
};

#endif

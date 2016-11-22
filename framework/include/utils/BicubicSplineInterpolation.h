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
 */
class BicubicSplineInterpolation : SplineInterpolationBase
{
public:
  BicubicSplineInterpolation();
  /**
   * If yp1, ypn are not specified or greater or equal that 1e30, we use natural spline
   */
  BicubicSplineInterpolation(const std::vector<Real> & x1, const std::vector<Real> & x2, const std::vector<std::vector<Real> > & y, Real yp1 = 1e30, Real ypn = 1e30);

  virtual ~BicubicSplineInterpolation() = default;

  /**
   * Set the x1-, x2, y- values and first derivatives
   */
  void setData(const std::vector<Real> & x1, const std::vector<Real> & x2, const std::vector<std::vector<Real> > & y, Real yp1 = 1e30, Real ypn = 1e30);

  void errorCheck();

  Real sample(Real x1, Real x2);
  Real sampleDerivative(Real x1, Real x2, unsigned int deriv_var);

protected:
  std::vector<Real> _x1;
  std::vector<Real> _x2;
  std::vector<std::vector<Real> > _y;
  /// boundary conditions
  Real _yp1, _ypn;
  /// Second derivatives
  std::vector<std::vector<Real> > _y2_rows;
  std::vector<std::vector<Real> > _y2_columns;

  void constructRowSplineSecondDerivativeTable();
  void constructColumnSplineSecondDerivativeTable();
  void solve();

  static int _file_number;
};

#endif

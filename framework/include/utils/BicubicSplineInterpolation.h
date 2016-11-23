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
 * Consistent with the terminology in Numerical Recipes, moving over a column spline means moving over the x1 coord
 * Likewise, moving over a row spline means moving over the x2 coord
 */
class BicubicSplineInterpolation : SplineInterpolationBase
{
public:
  BicubicSplineInterpolation();
  /**
   * In the future, may add interface that allows necessary vector of boundary conditions to be supplied for each edge of grid;
   * however, for now we just use natural splines at the grid boundaries
   */
  BicubicSplineInterpolation(const std::vector<Real> & x1, const std::vector<Real> & x2, const std::vector<std::vector<Real> > & y);

  virtual ~BicubicSplineInterpolation() = default;

  /**
   * Set the x1-, x2, y- values and first derivatives
   */
  void setData(const std::vector<Real> & x1, const std::vector<Real> & x2, const std::vector<std::vector<Real> > & y);

  void errorCheck();

  Real sample(Real x1, Real x2);
  Real sampleDerivative(Real x1, Real x2, unsigned int deriv_var);
  Real sample2ndDerivative(Real x1, Real x2, unsigned int deriv_var);

protected:
  std::vector<Real> _x1;
  std::vector<Real> _x2;
  std::vector<std::vector<Real> > _y;

  /// Second derivatives
  std::vector<std::vector<Real> > _y2_rows;
  std::vector<std::vector<Real> > _y2_columns;

  void constructRowSplineSecondDerivativeTable();
  void constructColumnSplineSecondDerivativeTable();
  void solve();

  static int _file_number;
};

#endif

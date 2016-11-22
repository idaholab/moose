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

#include "BicubicSplineInterpolation.h"
#include "MooseError.h"

int BicubicSplineInterpolation::_file_number = 0;

BicubicSplineInterpolation::BicubicSplineInterpolation()
{
}

BicubicSplineInterpolation::BicubicSplineInterpolation(const std::vector<Real> & x1, const std::vector<Real> & x2, const std::vector<std::vector<Real> > & y, Real yp1/* = 1e30*/, Real ypn/* = 1e30*/) :
    SplineInterpolationBase(),
    _x1(x1),
    _x2(x2),
    _y(y),
    _yp1(yp1),
    _ypn(ypn)
{
  errorCheck();
  solve();
}

void
BicubicSplineInterpolation::setData(const std::vector<Real> & x1, const std::vector<Real> & x2, const std::vector<std::vector<Real> > & y, Real yp1/* = 1e30*/, Real ypn/* = 1e30*/)
{
  _x1 = x1;
  _x2 = x2;
  _y = y;
  _yp1 = yp1;
  _ypn = ypn;
  errorCheck();
  solve();
}

void
BicubicSplineInterpolation::errorCheck()
{
  if (_x1.size() != _y.size())
    mooseError("y row dimension does not match the size of x1.");
  else
    for (int i = 0; i < _y.size(); ++i)
      if (_y[i].size() != _x2.size())
        mooseError("y column dimension does not match the size of x2.");
}

void
BicubicSplineInterpolation::constructRowSplineSecondDerivativeTable()
{
  int m = _x1.size();
  _y2_rows.resize(m);

  for (int j = 0; j < m; ++j)
    spline(_x2, _y[j], _y2_rows[j]);
}

void
BicubicSplineInterpolation::constructColumnSplineSecondDerivativeTable()
{
  int n = _x2.size();
  _y2_columns.resize(n);

  for (int j = 0; j < n; ++j)
    spline(_x1, _y[j], _y2_columns[j]);
}

void
BicubicSplineInterpolation::solve()
{
  constructRowSplineSecondDerivativeTable();
  constructColumnSplineSecondDerivativeTable();
}

Real
BicubicSplineInterpolation::sample(Real x1, Real x2)
{
  int m = _x1.size();
  std::vector<Real> column_spline_second_derivs(m), row_spline_eval(m);

  // Evaluate m row-splines to get y-values for column spline construction
  for (int j = 0; j < m; ++j)
    row_spline_eval[j] = SplineInterpolationBase::sample(_x2, _y[j], _y2_rows[j], x2);

  // Construct single column spline; get back the second derivatives
  spline(_x1, row_spline_eval, column_spline_second_derivs);

  // Evaluate newly constructed column spline
  return SplineInterpolationBase::sample(_x1, row_spline_eval, column_spline_second_derivs, x1);
}

Real
BicubicSplineInterpolation::sampleDerivative(Real x1, Real x2, unsigned int deriv_var)
{
  if (deriv_var == 1)
  {
    // Take derivative along x1 axis
    int m = _x1.size();
    std::vector<Real> column_spline_second_derivs(m), row_spline_eval(m);

    // Evaluate m row-splines to get y-values for column spline construction
    for (int j = 0; j < m; ++j)
      row_spline_eval[j] = SplineInterpolationBase::sample(_x2, _y[j], _y2_rows[j], x2);

    // Construct single column spline; get back the second derivatives
    spline(_x1, row_spline_eval, column_spline_second_derivs);

    // Evaluate derivative of newly constructed column spline
    return SplineInterpolationBase::sampleDerivative(_x1, row_spline_eval, column_spline_second_derivs, x1);
  }

  else if (deriv_var == 2)
  {
    // Take derivative along x1 axis
    int n = _x2.size();
    std::vector<Real> row_spline_second_derivs(n), column_spline_eval(n);

    // Evaluate n column-splines to get y-values for row spline construction
    for (int j = 0; j < n; ++j)
      column_spline_eval[j] = SplineInterpolationBase::sample(_x1, _y[j], _y2_columns[j], x1);

    // Construct single row spline; get back the second derivatives
    spline(_x2, column_spline_eval, row_spline_second_derivs);

    // Evaluate derivative of newly constructed row spline
    return SplineInterpolationBase::sampleDerivative(_x2, column_spline_eval, row_spline_second_derivs, x2);
  }

  else
  {
    mooseError("deriv_var must be either 1 or 2");
    return 0;
  }

}

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

BicubicSplineInterpolation::BicubicSplineInterpolation(const std::vector<Real> & x1, const std::vector<Real> & x2, const std::vector<std::vector<Real> > & y) :
    SplineInterpolationBase(),
    _x1(x1),
    _x2(x2),
    _y(y)
{
  errorCheck();
  solve();
}

void
BicubicSplineInterpolation::setData(const std::vector<Real> & x1, const std::vector<Real> & x2, const std::vector<std::vector<Real> > & y)
{
  _x1 = x1;
  _x2 = x2;
  _y = y;
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

  // Construct single column spline; get back the second derivatives wrt x1 coord on the x1 grid points
  spline(_x1, row_spline_eval, column_spline_second_derivs);

  // Evaluate newly constructed column spline
  return SplineInterpolationBase::sample(_x1, row_spline_eval, column_spline_second_derivs, x1);
}

Real
BicubicSplineInterpolation::sampleDerivative(Real x1, Real x2, unsigned int deriv_var)
{
  // Take derivative along x1 axis
  if (deriv_var == 1)
  {
    int m = _x1.size();
    std::vector<Real> column_spline_second_derivs(m), row_spline_eval(m);

    // Evaluate m row-splines to get y-values for column spline construction
    for (int j = 0; j < m; ++j)
      row_spline_eval[j] = SplineInterpolationBase::sample(_x2, _y[j], _y2_rows[j], x2);

    // Construct single column spline; get back the second derivatives wrt x1 coord on the x1 grid points
    spline(_x1, row_spline_eval, column_spline_second_derivs);

    // Evaluate derivative wrt x1 of newly constructed column spline
    return SplineInterpolationBase::sampleDerivative(_x1, row_spline_eval, column_spline_second_derivs, x1);
  }

  // Take derivative along x2 axis
  else if (deriv_var == 2)
  {
    int n = _x2.size();
    std::vector<Real> row_spline_second_derivs(n), column_spline_eval(n);

    // Evaluate n column-splines to get y-values for row spline construction
    for (int j = 0; j < n; ++j)
      column_spline_eval[j] = SplineInterpolationBase::sample(_x1, _y[j], _y2_columns[j], x1);

    // Construct single row spline; get back the second derivatives wrt x2 coord on the x2 grid points
    spline(_x2, column_spline_eval, row_spline_second_derivs);

    // Evaluate derivative wrt x2 of newly constructed row spline
    return SplineInterpolationBase::sampleDerivative(_x2, column_spline_eval, row_spline_second_derivs, x2);
  }

  else
  {
    mooseError("deriv_var must be either 1 or 2");
    return 0;
  }
}

Real
BicubicSplineInterpolation::sample2ndDerivative(Real x1, Real x2, unsigned int deriv_var)
{
  // Take second derivative along x1 axis
  if (deriv_var == 1)
  {
    int m = _x1.size();
    std::vector<Real> column_spline_second_derivs(m), row_spline_eval(m);

    // Evaluate m row-splines to get y-values for column spline construction
    for (int j = 0; j < m; ++j)
      row_spline_eval[j] = SplineInterpolationBase::sample(_x2, _y[j], _y2_rows[j], x2);

    // Construct single column spline; get back the second derivatives wrt x1 coord on the x1 grid points
    spline(_x1, row_spline_eval, column_spline_second_derivs);

    // Evaluate second derivative wrt x1 of newly constructed column spline
    return SplineInterpolationBase::sampleDerivative(_x1, row_spline_eval, column_spline_second_derivs, x1);
  }

  // Take second derivative along x2 axis
  else if (deriv_var == 2)
  {
    int n = _x2.size();
    std::vector<Real> row_spline_second_derivs(n), column_spline_eval(n);

    // Evaluate n column-splines to get y-values for row spline construction
    for (int j = 0; j < n; ++j)
      column_spline_eval[j] = SplineInterpolationBase::sample(_x1, _y[j], _y2_columns[j], x1);

    // Construct single row spline; get back the second derivatives wrt x2 coord on the x2 grid points
    spline(_x2, column_spline_eval, row_spline_second_derivs);

    // Evaluate second derivative wrt x2 of newly constructed row spline
    return SplineInterpolationBase::sampleDerivative(_x2, column_spline_eval, row_spline_second_derivs, x2);
  }

  else
  {
    mooseError("deriv_var must be either 1 or 2");
    return 0;
  }
}

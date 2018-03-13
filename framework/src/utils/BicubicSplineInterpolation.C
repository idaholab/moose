//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BicubicSplineInterpolation.h"
#include "MooseError.h"

int BicubicSplineInterpolation::_file_number = 0;

BicubicSplineInterpolation::BicubicSplineInterpolation() {}

BicubicSplineInterpolation::BicubicSplineInterpolation(const std::vector<Real> & x1,
                                                       const std::vector<Real> & x2,
                                                       const std::vector<std::vector<Real>> & y,
                                                       const std::vector<Real> & yx11,
                                                       const std::vector<Real> & yx1n,
                                                       const std::vector<Real> & yx21,
                                                       const std::vector<Real> & yx2n)
  : SplineInterpolationBase(),
    _x1(x1),
    _x2(x2),
    _y(y),
    _yx11(yx11),
    _yx1n(yx1n),
    _yx21(yx21),
    _yx2n(yx2n)
{
  auto n = _x2.size();
  _row_spline_second_derivs.resize(n);
  _column_spline_eval.resize(n);

  auto m = _x1.size();
  _column_spline_second_derivs.resize(m);
  _row_spline_eval.resize(m);

  errorCheck();
  solve();
}

void
BicubicSplineInterpolation::setData(const std::vector<Real> & x1,
                                    const std::vector<Real> & x2,
                                    const std::vector<std::vector<Real>> & y,
                                    const std::vector<Real> & yx11,
                                    const std::vector<Real> & yx1n,
                                    const std::vector<Real> & yx21,
                                    const std::vector<Real> & yx2n)
{
  _x1 = x1;
  _x2 = x2;
  _y = y;
  _yx11 = yx11;
  _yx1n = yx1n;
  _yx21 = yx21;
  _yx2n = yx2n;

  auto n = _x2.size();
  _row_spline_second_derivs.resize(n);
  _column_spline_eval.resize(n);

  auto m = _x1.size();
  _column_spline_second_derivs.resize(m);
  _row_spline_eval.resize(m);

  errorCheck();
  solve();
}

void
BicubicSplineInterpolation::errorCheck()
{
  auto m = _x1.size(), n = _x2.size();

  if (_y.size() != m)
    mooseError("y row dimension does not match the size of x1.");
  else
    for (decltype(m) i = 0; i < _y.size(); ++i)
      if (_y[i].size() != n)
        mooseError("y column dimension does not match the size of x2.");

  if (_yx11.empty())
    _yx11.resize(n, _deriv_bound);
  else if (_yx11.size() != n)
    mooseError("The length of the vectors holding the first derivatives of y with respect to x1 "
               "must match the length of x2.");

  if (_yx1n.empty())
    _yx1n.resize(n, _deriv_bound);
  else if (_yx1n.size() != n)
    mooseError("The length of the vectors holding the first derivatives of y with respect to x1 "
               "must match the length of x2.");

  if (_yx21.empty())
    _yx21.resize(m, _deriv_bound);
  else if (_yx21.size() != m)
    mooseError("The length of the vectors holding the first derivatives of y with respect to x2 "
               "must match the length of x1.");

  if (_yx2n.empty())
    _yx2n.resize(m, _deriv_bound);
  else if (_yx2n.size() != m)
    mooseError("The length of the vectors holding the first derivatives of y with respect to x2 "
               "must match the length of x1.");
}

void
BicubicSplineInterpolation::constructRowSplineSecondDerivativeTable()
{
  auto m = _x1.size();
  _y2_rows.resize(m);

  if (_yx21.empty())
    for (decltype(m) j = 0; j < m; ++j)
      spline(_x2, _y[j], _y2_rows[j]);

  else
    for (decltype(m) j = 0; j < m; ++j)
      spline(_x2, _y[j], _y2_rows[j], _yx21[j], _yx2n[j]);
}

void
BicubicSplineInterpolation::constructColumnSplineSecondDerivativeTable()
{
  auto m = _x1.size();
  auto n = _x2.size();
  _y2_columns.resize(n);
  _y_trans.resize(n);

  for (decltype(n) j = 0; j < n; ++j)
    _y_trans[j].resize(m);

  // transpose the _y values so the columns can be easily iterated over
  for (decltype(n) i = 0; i < _y.size(); ++i)
    for (decltype(n) j = 0; j < _y[0].size(); ++j)
      _y_trans[j][i] = _y[i][j];

  if (_yx11.empty())
    for (decltype(n) j = 0; j < n; ++j)
      spline(_x1, _y_trans[j], _y2_columns[j]);

  else
    for (decltype(n) j = 0; j < n; ++j)
      spline(_x1, _y_trans[j], _y2_columns[j], _yx11[j], _yx1n[j]);
}

void
BicubicSplineInterpolation::solve()
{
  constructRowSplineSecondDerivativeTable();
  constructColumnSplineSecondDerivativeTable();
}

Real
BicubicSplineInterpolation::sample(Real x1,
                                   Real x2,
                                   Real yx11 /* = _deriv_bound*/,
                                   Real yx1n /* = _deriv_bound*/)
{
  constructColumnSpline(x2, _row_spline_eval, _column_spline_second_derivs, yx11, yx1n);

  // Evaluate newly constructed column spline
  return SplineInterpolationBase::sample(_x1, _row_spline_eval, _column_spline_second_derivs, x1);
}

Real
BicubicSplineInterpolation::sampleDerivative(Real x1,
                                             Real x2,
                                             unsigned int deriv_var,
                                             Real yp1 /* = _deriv_bound*/,
                                             Real ypn /* = _deriv_bound*/)
{
  // Take derivative along x1 axis
  if (deriv_var == 1)
  {
    constructColumnSpline(x2, _row_spline_eval, _column_spline_second_derivs, yp1, ypn);

    // Evaluate derivative wrt x1 of newly constructed column spline
    return SplineInterpolationBase::sampleDerivative(
        _x1, _row_spline_eval, _column_spline_second_derivs, x1);
  }

  // Take derivative along x2 axis
  else if (deriv_var == 2)
  {
    constructRowSpline(x1, _column_spline_eval, _row_spline_second_derivs, yp1, ypn);

    // Evaluate derivative wrt x2 of newly constructed row spline
    return SplineInterpolationBase::sampleDerivative(
        _x2, _column_spline_eval, _row_spline_second_derivs, x2);
  }

  else
    mooseError("deriv_var must be either 1 or 2 in BicubicSplineInterpolation");
}

Real
BicubicSplineInterpolation::sample2ndDerivative(Real x1,
                                                Real x2,
                                                unsigned int deriv_var,
                                                Real yp1 /* = _deriv_bound*/,
                                                Real ypn /* = _deriv_bound*/)
{
  // Take second derivative along x1 axis
  if (deriv_var == 1)
  {
    constructColumnSpline(x2, _row_spline_eval, _column_spline_second_derivs, yp1, ypn);

    // Evaluate second derivative wrt x1 of newly constructed column spline
    return SplineInterpolationBase::sample2ndDerivative(
        _x1, _row_spline_eval, _column_spline_second_derivs, x1);
  }

  // Take second derivative along x2 axis
  else if (deriv_var == 2)
  {
    constructRowSpline(x1, _column_spline_eval, _row_spline_second_derivs, yp1, ypn);

    // Evaluate second derivative wrt x2 of newly constructed row spline
    return SplineInterpolationBase::sample2ndDerivative(
        _x2, _column_spline_eval, _row_spline_second_derivs, x2);
  }

  else
    mooseError("deriv_var must be either 1 or 2 in BicubicSplineInterpolation");
}

void
BicubicSplineInterpolation::sampleValueAndDerivatives(Real x1,
                                                      Real x2,
                                                      Real & y,
                                                      Real & dy1,
                                                      Real & dy2,
                                                      Real yx11 /* = _deriv_bound*/,
                                                      Real yx1n /* = _deriv_bound*/,
                                                      Real yx21 /* = _deriv_bound*/,
                                                      Real yx2n /* = _deriv_bound*/)
{
  constructColumnSpline(x2, _row_spline_eval, _column_spline_second_derivs, yx11, yx1n);
  y = SplineInterpolationBase::sample(_x1, _row_spline_eval, _column_spline_second_derivs, x1);
  dy1 = SplineInterpolationBase::sampleDerivative(
      _x1, _row_spline_eval, _column_spline_second_derivs, x1);

  constructRowSpline(x1, _column_spline_eval, _row_spline_second_derivs, yx21, yx2n);
  dy2 = SplineInterpolationBase::sampleDerivative(
      _x2, _column_spline_eval, _row_spline_second_derivs, x2);
}

void
BicubicSplineInterpolation::constructRowSpline(Real x1,
                                               std::vector<Real> & column_spline_eval,
                                               std::vector<Real> & row_spline_second_derivs,
                                               Real yx11 /*= _deriv_bound*/,
                                               Real yx1n /*= _deriv_bound*/)
{
  auto n = _x2.size();

  // Find the indices that bound the point x1
  unsigned int klo, khi;
  findInterval(_x1, x1, klo, khi);

  // Evaluate n column-splines to get y-values for row spline construction using
  // the indices above to avoid computing them for each j
  for (decltype(n) j = 0; j < n; ++j)
    _column_spline_eval[j] =
        SplineInterpolationBase::sample(_x1, _y_trans[j], _y2_columns[j], x1, klo, khi);

  // Construct single row spline; get back the second derivatives wrt x2 coord
  // on the x2 grid points
  spline(_x2, column_spline_eval, row_spline_second_derivs, yx11, yx1n);
}

void
BicubicSplineInterpolation::constructColumnSpline(Real x2,
                                                  std::vector<Real> & row_spline_eval,
                                                  std::vector<Real> & column_spline_second_derivs,
                                                  Real yx21 /*= _deriv_bound*/,
                                                  Real yx2n /*= _deriv_bound*/)
{
  auto m = _x1.size();

  // Find the indices that bound the point x2
  unsigned int klo, khi;
  findInterval(_x2, x2, klo, khi);

  // Evaluate m row-splines to get y-values for column spline construction using
  // the indices above to avoid computing them for each j
  for (decltype(m) j = 0; j < m; ++j)
    _row_spline_eval[j] = SplineInterpolationBase::sample(_x2, _y[j], _y2_rows[j], x2, klo, khi);

  // Construct single column spline; get back the second derivatives wrt x1 coord
  // on the x1 grid points
  spline(_x1, row_spline_eval, column_spline_second_derivs, yx21, yx2n);
}

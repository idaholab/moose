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
  auto n = _x2.size();
  _y2_columns.resize(n);

  if (_yx11.empty())
    for (decltype(n) j = 0; j < n; ++j)
      spline(_x1, _y[j], _y2_columns[j]);

  else
    for (decltype(n) j = 0; j < n; ++j)
      spline(_x1, _y[j], _y2_columns[j], _yx11[j], _yx1n[j]);
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
  auto m = _x1.size();
  std::vector<Real> column_spline_second_derivs(m), row_spline_eval(m);

  // Evaluate m row-splines to get y-values for column spline construction
  for (decltype(m) j = 0; j < m; ++j)
    row_spline_eval[j] = SplineInterpolationBase::sample(_x2, _y[j], _y2_rows[j], x2);

  // Construct single column spline; get back the second derivatives wrt x1 coord on the x1 grid
  // points
  spline(_x1, row_spline_eval, column_spline_second_derivs, yx11, yx1n);

  // Evaluate newly constructed column spline
  return SplineInterpolationBase::sample(_x1, row_spline_eval, column_spline_second_derivs, x1);
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
    auto m = _x1.size();
    std::vector<Real> column_spline_second_derivs(m), row_spline_eval(m);

    // Evaluate m row-splines to get y-values for column spline construction
    for (decltype(m) j = 0; j < m; ++j)
      row_spline_eval[j] = SplineInterpolationBase::sample(_x2, _y[j], _y2_rows[j], x2);

    // Construct single column spline; get back the second derivatives wrt x1 coord on the x1 grid
    // points
    spline(_x1, row_spline_eval, column_spline_second_derivs, yp1, ypn);

    // Evaluate derivative wrt x1 of newly constructed column spline
    return SplineInterpolationBase::sampleDerivative(
        _x1, row_spline_eval, column_spline_second_derivs, x1);
  }

  // Take derivative along x2 axis
  else if (deriv_var == 2)
  {
    auto n = _x2.size();
    std::vector<Real> row_spline_second_derivs(n), column_spline_eval(n);

    // Evaluate n column-splines to get y-values for row spline construction
    for (decltype(n) j = 0; j < n; ++j)
      column_spline_eval[j] = SplineInterpolationBase::sample(_x1, _y[j], _y2_columns[j], x1);

    // Construct single row spline; get back the second derivatives wrt x2 coord on the x2 grid
    // points
    spline(_x2, column_spline_eval, row_spline_second_derivs, yp1, ypn);

    // Evaluate derivative wrt x2 of newly constructed row spline
    return SplineInterpolationBase::sampleDerivative(
        _x2, column_spline_eval, row_spline_second_derivs, x2);
  }

  else
  {
    mooseError("deriv_var must be either 1 or 2");
    return 0;
  }
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
    auto m = _x1.size();
    std::vector<Real> column_spline_second_derivs(m), row_spline_eval(m);

    // Evaluate m row-splines to get y-values for column spline construction
    for (decltype(m) j = 0; j < m; ++j)
      row_spline_eval[j] = SplineInterpolationBase::sample(_x2, _y[j], _y2_rows[j], x2);

    // Construct single column spline; get back the second derivatives wrt x1 coord on the x1 grid
    // points
    spline(_x1, row_spline_eval, column_spline_second_derivs, yp1, ypn);

    // Evaluate second derivative wrt x1 of newly constructed column spline
    return SplineInterpolationBase::sampleDerivative(
        _x1, row_spline_eval, column_spline_second_derivs, x1);
  }

  // Take second derivative along x2 axis
  else if (deriv_var == 2)
  {
    auto n = _x2.size();
    std::vector<Real> row_spline_second_derivs(n), column_spline_eval(n);

    // Evaluate n column-splines to get y-values for row spline construction
    for (decltype(n) j = 0; j < n; ++j)
      column_spline_eval[j] = SplineInterpolationBase::sample(_x1, _y[j], _y2_columns[j], x1);

    // Construct single row spline; get back the second derivatives wrt x2 coord on the x2 grid
    // points
    spline(_x2, column_spline_eval, row_spline_second_derivs, yp1, ypn);

    // Evaluate second derivative wrt x2 of newly constructed row spline
    return SplineInterpolationBase::sampleDerivative(
        _x2, column_spline_eval, row_spline_second_derivs, x2);
  }

  else
  {
    mooseError("deriv_var must be either 1 or 2");
    return 0;
  }
}

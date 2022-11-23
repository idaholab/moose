//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BicubicInterpolation.h"
#include "MooseError.h"
#include "MathUtils.h"
#include "MooseUtils.h"

#include "libmesh/int_range.h"

BicubicInterpolation::BicubicInterpolation(const std::vector<Real> & x1,
                                           const std::vector<Real> & x2,
                                           const std::vector<std::vector<Real>> & y)
  : BidimensionalInterpolation(x1, x2), _y(y)
{
  errorCheck();

  auto m = _x1.size();
  auto n = _x2.size();

  _bicubic_coeffs.resize(m - 1);
  for (const auto i : index_range(_bicubic_coeffs))
  {
    _bicubic_coeffs[i].resize(n - 1);
    for (const auto j : index_range(_bicubic_coeffs[i]))
    {
      _bicubic_coeffs[i][j].resize(4);
      for (const auto k : make_range(4))
        _bicubic_coeffs[i][j][k].resize(4);
    }
  }

  // Precompute the coefficients
  precomputeCoefficients();
}

Real
BicubicInterpolation::sample(const Real x1, const Real x2) const
{
  return sampleInternal(x1, x2);
}

ADReal
BicubicInterpolation::sample(const ADReal & x1, const ADReal & x2) const
{
  return sampleInternal(x1, x2);
}

template <class C>
C
BicubicInterpolation::sampleInternal(const C & x1, const C & x2) const
{
  unsigned int x1l, x1u, x2l, x2u;
  C t, u;
  findInterval(_x1, x1, x1l, x1u, t);
  findInterval(_x2, x2, x2l, x2u, u);

  C sample = 0.0;
  for (const auto i : make_range(4))
    for (const auto j : make_range(4))
      sample += _bicubic_coeffs[x1l][x2l][i][j] * MathUtils::pow(t, i) * MathUtils::pow(u, j);

  return sample;
}

Real
BicubicInterpolation::sampleDerivative(Real x1, Real x2, unsigned int deriv_var) const
{
  unsigned int x1l, x1u, x2l, x2u;
  Real t, u;
  findInterval(_x1, x1, x1l, x1u, t);
  findInterval(_x2, x2, x2l, x2u, u);

  // Take derivative along x1 axis
  // Note: sum from i = 1 as the first term is zero
  if (deriv_var == 1)
  {
    Real sample_deriv = 0.0;
    for (const auto i : make_range(1, 4))
      for (const auto j : make_range(4))
        sample_deriv +=
            i * _bicubic_coeffs[x1l][x2l][i][j] * MathUtils::pow(t, i - 1) * MathUtils::pow(u, j);

    Real d = _x1[x1u] - _x1[x1l];

    if (!MooseUtils::absoluteFuzzyEqual(d, 0.0))
      sample_deriv /= d;

    return sample_deriv;
  }

  // Take derivative along x2 axis
  // Note: sum from j = 1 as the first term is zero
  else if (deriv_var == 2)
  {
    Real sample_deriv = 0.0;

    for (const auto i : make_range(4))
      for (const auto j : make_range(1, 4))
        sample_deriv +=
            j * _bicubic_coeffs[x1l][x2l][i][j] * MathUtils::pow(t, i) * MathUtils::pow(u, j - 1);

    Real d = _x2[x2u] - _x2[x2l];

    if (!MooseUtils::absoluteFuzzyEqual(d, Real(0.0)))
      sample_deriv /= d;

    return sample_deriv;
  }

  else
    mooseError("deriv_var must be either 1 or 2 in BicubicInterpolation");
}

Real
BicubicInterpolation::sample2ndDerivative(Real x1, Real x2, unsigned int deriv_var) const
{
  unsigned int x1l, x1u, x2l, x2u;
  Real t, u;
  findInterval(_x1, x1, x1l, x1u, t);
  findInterval(_x2, x2, x2l, x2u, u);

  // Take derivative along x1 axis
  // Note: sum from i = 2 as the first two terms are zero
  if (deriv_var == 1)
  {
    Real sample_deriv = 0.0;
    for (const auto i : make_range(2, 4))
      for (const auto j : make_range(4))
        sample_deriv += i * (i - 1) * _bicubic_coeffs[x1l][x2l][i][j] * MathUtils::pow(t, i - 2) *
                        MathUtils::pow(u, j);

    Real d = _x1[x1u] - _x1[x1l];

    if (!MooseUtils::absoluteFuzzyEqual(d, Real(0.0)))
      sample_deriv /= (d * d);

    return sample_deriv;
  }

  // Take derivative along x2 axis
  // Note: sum from j = 2 as the first two terms are zero
  else if (deriv_var == 2)
  {
    Real sample_deriv = 0.0;
    for (const auto i : make_range(4))
      for (const auto j : make_range(2, 4))
        sample_deriv += j * (j - 1) * _bicubic_coeffs[x1l][x2l][i][j] * MathUtils::pow(t, i) *
                        MathUtils::pow(u, j - 2);

    Real d = _x2[x2u] - _x2[x2l];

    if (!MooseUtils::absoluteFuzzyEqual(d, Real(0.0)))
      sample_deriv /= (d * d);

    return sample_deriv;
  }

  else
    mooseError("deriv_var must be either 1 or 2 in BicubicInterpolation");
}

void
BicubicInterpolation::sampleValueAndDerivatives(
    Real x1, Real x2, Real & y, Real & dy1, Real & dy2) const
{
  unsigned int x1l, x1u, x2l, x2u;
  Real t, u;
  findInterval(_x1, x1, x1l, x1u, t);
  findInterval(_x2, x2, x2l, x2u, u);

  y = 0.0;
  for (const auto i : make_range(4))
    for (const auto j : make_range(4))
    {
      y += _bicubic_coeffs[x1l][x2l][i][j] * MathUtils::pow(t, i) * MathUtils::pow(u, j);
    }
  // Note: sum from i = 1 as the first term is zero
  dy1 = 0.0;
  for (const auto i : make_range(1, 4))
    for (const auto j : make_range(4))
      dy1 += i * _bicubic_coeffs[x1l][x2l][i][j] * MathUtils::pow(t, i - 1) * MathUtils::pow(u, j);

  // Note: sum from j = 1 as the first term is zero
  dy2 = 0.0;
  for (const auto i : make_range(4))
    for (const auto j : make_range(1, 4))
      dy2 += j * _bicubic_coeffs[x1l][x2l][i][j] * MathUtils::pow(t, i) * MathUtils::pow(u, j - 1);

  Real d1 = _x1[x1u] - _x1[x1l];

  if (!MooseUtils::absoluteFuzzyEqual(d1, Real(0.0)))
    dy1 /= d1;

  Real d2 = _x2[x2u] - _x2[x2l];

  if (!MooseUtils::absoluteFuzzyEqual(d2, Real(0.0)))
    dy2 /= d2;
}

void
BicubicInterpolation::sampleValueAndDerivatives(
    const ADReal & x1, const ADReal & x2, ADReal & y, ADReal & dy1, ADReal & dy2) const
{
  unsigned int x1l, x1u, x2l, x2u;
  ADReal t, u;
  findInterval(_x1, x1, x1l, x1u, t);
  findInterval(_x2, x2, x2l, x2u, u);

  y = 0.0;
  for (const auto i : make_range(4))
    for (const auto j : make_range(4))
      y += _bicubic_coeffs[x1l][x2l][i][j] * MathUtils::pow(t, i) * MathUtils::pow(u, j);

  // Note: sum from i = 1 as the first term is zero
  dy1 = 0.0;
  for (const auto i : make_range(1, 4))
    for (const auto j : make_range(4))
      dy1 += i * _bicubic_coeffs[x1l][x2l][i][j] * MathUtils::pow(t, i - 1) * MathUtils::pow(u, j);

  // Note: sum from j = 1 as the first term is zero
  dy2 = 0.0;
  for (const auto i : make_range(4))
    for (const auto j : make_range(1, 4))
      dy2 += j * _bicubic_coeffs[x1l][x2l][i][j] * MathUtils::pow(t, i) * MathUtils::pow(u, j - 1);

  ADReal d1 = _x1[x1u] - _x1[x1l];

  if (!MooseUtils::absoluteFuzzyEqual(d1, ADReal(0.0)))
    dy1 /= d1;

  ADReal d2 = _x2[x2u] - _x2[x2l];

  if (!MooseUtils::absoluteFuzzyEqual(d2, ADReal(0.0)))
    dy2 /= d2;
}

void
BicubicInterpolation::precomputeCoefficients()
{
  // Calculate the first derivatives in each direction at each point, and the
  // mixed second derivative
  std::vector<std::vector<Real>> dy_dx1, dy_dx2, d2y_dx1x2;
  tableDerivatives(dy_dx1, dy_dx2, d2y_dx1x2);

  // Now solve for the coefficients at each point in the grid
  for (const auto i : index_range(_bicubic_coeffs))
    for (const auto j : index_range(_bicubic_coeffs[i]))
    {
      // Distance between corner points in each direction
      const Real d1 = _x1[i + 1] - _x1[i];
      const Real d2 = _x2[j + 1] - _x2[j];

      // Values of function and derivatives of the four corner points
      std::vector<Real> y = {_y[i][j], _y[i + 1][j], _y[i + 1][j + 1], _y[i][j + 1]};
      std::vector<Real> y1 = {
          dy_dx1[i][j], dy_dx1[i + 1][j], dy_dx1[i + 1][j + 1], dy_dx1[i][j + 1]};
      std::vector<Real> y2 = {
          dy_dx2[i][j], dy_dx2[i + 1][j], dy_dx2[i + 1][j + 1], dy_dx2[i][j + 1]};
      std::vector<Real> y12 = {
          d2y_dx1x2[i][j], d2y_dx1x2[i + 1][j], d2y_dx1x2[i + 1][j + 1], d2y_dx1x2[i][j + 1]};

      std::vector<Real> cl(16), x(16);
      Real xx;
      unsigned int count = 0;

      // Temporary vector used in the matrix multiplication
      for (const auto k : make_range(4))
      {
        x[k] = y[k];
        x[k + 4] = y1[k] * d1;
        x[k + 8] = y2[k] * d2;
        x[k + 12] = y12[k] * d1 * d2;
      }

      // Multiply by the matrix of constants
      for (const auto k : make_range(16))
      {
        xx = 0.0;
        for (const auto q : make_range(16))
          xx += _wt[k][q] * x[q];

        cl[k] = xx;
      }

      // Unpack results into coefficient table
      for (const auto k : make_range(4))
        for (const auto q : make_range(4))
        {
          _bicubic_coeffs[i][j][k][q] = cl[count];
          count++;
        }
    }
}

void
BicubicInterpolation::tableDerivatives(std::vector<std::vector<Real>> & dy_dx1,
                                       std::vector<std::vector<Real>> & dy_dx2,
                                       std::vector<std::vector<Real>> & d2y_dx1x2)
{
  const auto m = _x1.size();
  const auto n = _x2.size();
  dy_dx1.resize(m);
  dy_dx2.resize(m);
  d2y_dx1x2.resize(m);

  for (const auto i : make_range(m))
  {
    dy_dx1[i].resize(n);
    dy_dx2[i].resize(n);
    d2y_dx1x2[i].resize(n);
  }

  // Central difference calculations of derivatives
  for (const auto i : make_range(m))
    for (const auto j : make_range(n))
    {
      // Derivative wrt x1
      if (i == 0)
      {
        dy_dx1[i][j] = (_y[i + 1][j] - _y[i][j]) / (_x1[i + 1] - _x1[i]);
      }
      else if (i == m - 1)
        dy_dx1[i][j] = (_y[i][j] - _y[i - 1][j]) / (_x1[i] - _x1[i - 1]);
      else
        dy_dx1[i][j] = (_y[i + 1][j] - _y[i - 1][j]) / (_x1[i + 1] - _x1[i - 1]);

      // Derivative wrt x2
      if (j == 0)
        dy_dx2[i][j] = (_y[i][j + 1] - _y[i][j]) / (_x2[j + 1] - _x2[j]);
      else if (j == n - 1)
        dy_dx2[i][j] = (_y[i][j] - _y[i][j - 1]) / (_x2[j] - _x2[j - 1]);
      else
        dy_dx2[i][j] = (_y[i][j + 1] - _y[i][j - 1]) / (_x2[j + 1] - _x2[j - 1]);

      // Mixed derivative d2y/dx1x2
      if (i == 0 && j == 0)
        d2y_dx1x2[i][j] = (_y[i + 1][j + 1] - _y[i + 1][j] - _y[i][j + 1] + _y[i][j]) /
                          (_x1[i + 1] - _x1[i]) / (_x2[j + 1] - _x2[j]);
      else if (i == 0 && j == n - 1)
        d2y_dx1x2[i][j] = (_y[i + 1][j] - _y[i + 1][j - 1] - _y[i][j] + _y[i][j - 1]) /
                          (_x1[i + 1] - _x1[i]) / (_x2[j] - _x2[j - 1]);
      else if (i == m - 1 && j == 0)
        d2y_dx1x2[i][j] = (_y[i][j + 1] - _y[i][j] - _y[i - 1][j + 1] + _y[i - 1][j]) /
                          (_x1[i] - _x1[i - 1]) / (_x2[j + 1] - _x2[j]);
      else if (i == m - 1 && j == n - 1)
        d2y_dx1x2[i][j] = (_y[i][j] - _y[i][j - 1] - _y[i - 1][j] + _y[i - 1][j - 1]) /
                          (_x1[i] - _x1[i - 1]) / (_x2[j] - _x2[j - 1]);
      else if (i == 0)
        d2y_dx1x2[i][j] = (_y[i + 1][j + 1] - _y[i + 1][j - 1] - _y[i][j + 1] + _y[i][j - 1]) /
                          (_x1[i + 1] - _x1[i]) / (_x2[j + 1] - _x2[j - 1]);
      else if (i == m - 1)
        d2y_dx1x2[i][j] = (_y[i][j + 1] - _y[i][j - 1] - _y[i - 1][j + 1] + _y[i - 1][j - 1]) /
                          (_x1[i] - _x1[i - 1]) / (_x2[j + 1] - _x2[j - 1]);
      else if (j == 0)
        d2y_dx1x2[i][j] = (_y[i + 1][j + 1] - _y[i + 1][j] - _y[i - 1][j + 1] + _y[i - 1][j]) /
                          (_x1[i + 1] - _x1[i - 1]) / (_x2[j + 1] - _x2[j]);
      else if (j == n - 1)
        d2y_dx1x2[i][j] = (_y[i + 1][j] - _y[i + 1][j - 1] - _y[i - 1][j] + _y[i - 1][j - 1]) /
                          (_x1[i + 1] - _x1[i - 1]) / (_x2[j] - _x2[j - 1]);
      else
        d2y_dx1x2[i][j] =
            (_y[i + 1][j + 1] - _y[i + 1][j - 1] - _y[i - 1][j + 1] + _y[i - 1][j - 1]) /
            (_x1[i + 1] - _x1[i - 1]) / (_x2[j + 1] - _x2[j - 1]);
    }
}

template <class C>
void
BicubicInterpolation::findInterval(
    const std::vector<Real> & x, const C & xi, unsigned int & klo, unsigned int & khi, C & xs) const
{
  // Find the indices that bracket the point xi
  klo = 0;
  mooseAssert(x.size() >= 2,
              "There must be at least two points in the table in BicubicInterpolation");

  khi = x.size() - 1;
  while (khi - klo > 1)
  {
    unsigned int kmid = (khi + klo) / 2;
    if (x[kmid] > xi)
      khi = kmid;
    else
      klo = kmid;
  }

  // Now find the scaled position, normalized to [0,1]
  Real d = x[khi] - x[klo];
  xs = xi - x[klo];

  if (!MooseUtils::absoluteFuzzyEqual(d, Real(0.0)))
    xs /= d;
}

template void BicubicInterpolation::findInterval(const std::vector<Real> & x,
                                                 const Real & xi,
                                                 unsigned int & klo,
                                                 unsigned int & khi,
                                                 Real & xs) const;
template void BicubicInterpolation::findInterval(const std::vector<Real> & x,
                                                 const ADReal & xi,
                                                 unsigned int & klo,
                                                 unsigned int & khi,
                                                 ADReal & xs) const;

void
BicubicInterpolation::errorCheck()
{
  auto m = _x1.size(), n = _x2.size();

  if (_y.size() != m)
    mooseError("y row dimension does not match the size of x1 in BicubicInterpolation");
  else
    for (const auto i : index_range(_y))
      if (_y[i].size() != n)
        mooseError("y column dimension does not match the size of x2 in BicubicInterpolation");
}

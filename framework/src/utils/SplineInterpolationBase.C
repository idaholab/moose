//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SplineInterpolationBase.h"
#include "MooseError.h"
#include <limits>

const Real SplineInterpolationBase::_deriv_bound = std::numeric_limits<Real>::max();

SplineInterpolationBase::SplineInterpolationBase() {}

void
SplineInterpolationBase::spline(const std::vector<Real> & x,
                                const std::vector<Real> & y,
                                std::vector<Real> & y2,
                                Real yp1 /* = _deriv_bound*/,
                                Real ypn /* = _deriv_bound*/)
{
  auto n = x.size();
  if (n < 2)
    mooseError("You must have at least two knots to create a spline.");

  std::vector<Real> u(n, 0.);
  y2.assign(n, 0.);

  if (yp1 >= 1e30)
    y2[0] = u[0] = 0.;
  else
  {
    y2[0] = -0.5;
    u[0] = (3.0 / (x[1] - x[0])) * ((y[1] - y[0]) / (x[1] - x[0]) - yp1);
  }
  // decomposition of tri-diagonal algorithm (y2 and u are used for temporary storage)
  for (decltype(n) i = 1; i < n - 1; i++)
  {
    Real sig = (x[i] - x[i - 1]) / (x[i + 1] - x[i - 1]);
    Real p = sig * y2[i - 1] + 2.0;
    y2[i] = (sig - 1.0) / p;
    u[i] = (y[i + 1] - y[i]) / (x[i + 1] - x[i]) - (y[i] - y[i - 1]) / (x[i] - x[i - 1]);
    u[i] = (6.0 * u[i] / (x[i + 1] - x[i - 1]) - sig * u[i - 1]) / p;
  }

  Real qn, un;
  if (ypn >= 1e30)
    qn = un = 0.;
  else
  {
    qn = 0.5;
    un = (3.0 / (x[n - 1] - x[n - 2])) * (ypn - (y[n - 1] - y[n - 2]) / (x[n - 1] - x[n - 2]));
  }

  y2[n - 1] = (un - qn * u[n - 2]) / (qn * y2[n - 2] + 1.);
  // back substitution
  for (auto k = n - 1; k >= 1; k--)
    y2[k - 1] = y2[k - 1] * y2[k] + u[k - 1];
}

void
SplineInterpolationBase::findInterval(const std::vector<Real> & x,
                                      Real x_int,
                                      unsigned int & klo,
                                      unsigned int & khi) const
{
  klo = 0;
  mooseAssert(x.size() >= 2, "You must have at least two knots to create a spline.");
  khi = x.size() - 1;
  while (khi - klo > 1)
  {
    unsigned int k = (khi + klo) >> 1;
    if (x[k] > x_int)
      khi = k;
    else
      klo = k;
  }
}

template <typename T>
void
SplineInterpolationBase::computeCoeffs(const std::vector<Real> & x,
                                       unsigned int klo,
                                       unsigned int khi,
                                       const T & x_int,
                                       Real & h,
                                       T & a,
                                       T & b) const
{
  h = x[khi] - x[klo];
  if (h == 0)
    mooseError("The values of x must be distinct");
  a = (x[khi] - x_int) / h;
  b = (x_int - x[klo]) / h;
}

Real
SplineInterpolationBase::sample(const std::vector<Real> & x,
                                const std::vector<Real> & y,
                                const std::vector<Real> & y2,
                                Real x_int) const
{
  unsigned int klo, khi;
  findInterval(x, x_int, klo, khi);

  return sample(x, y, y2, x_int, klo, khi);
}

ADReal
SplineInterpolationBase::sample(const std::vector<Real> & x,
                                const std::vector<Real> & y,
                                const std::vector<Real> & y2,
                                const ADReal & x_int) const
{
  unsigned int klo, khi;
  findInterval(x, MetaPhysicL::raw_value(x_int), klo, khi);

  return sample(x, y, y2, x_int, klo, khi);
}

Real
SplineInterpolationBase::sampleDerivative(const std::vector<Real> & x,
                                          const std::vector<Real> & y,
                                          const std::vector<Real> & y2,
                                          Real x_int) const
{
  unsigned int klo, khi;
  findInterval(x, x_int, klo, khi);

  Real h, a, b;
  computeCoeffs(x, klo, khi, x_int, h, a, b);

  return (y[khi] - y[klo]) / h -
         (((3.0 * a * a - 1.0) * y2[klo] + (3.0 * b * b - 1.0) * -y2[khi]) * h / 6.0);
}

Real
SplineInterpolationBase::sample2ndDerivative(const std::vector<Real> & x,
                                             const std::vector<Real> & /*y*/,
                                             const std::vector<Real> & y2,
                                             Real x_int) const
{
  unsigned int klo, khi;
  findInterval(x, x_int, klo, khi);

  Real h, a, b;
  computeCoeffs(x, klo, khi, x_int, h, a, b);

  return a * y2[klo] + b * y2[khi];
}

template <typename T>
T
SplineInterpolationBase::sample(const std::vector<Real> & x,
                                const std::vector<Real> & y,
                                const std::vector<Real> & y2,
                                const T & x_int,
                                unsigned int klo,
                                unsigned int khi) const
{
  Real h;
  T a, b;
  computeCoeffs(x, klo, khi, x_int, h, a, b);

  return a * y[klo] + b * y[khi] +
         ((a * a * a - a) * y2[klo] + (b * b * b - b) * y2[khi]) * (h * h) / 6.0;
}

template Real SplineInterpolationBase::sample<Real>(const std::vector<Real> & x,
                                                    const std::vector<Real> & y,
                                                    const std::vector<Real> & y2,
                                                    const Real & x_int,
                                                    unsigned int klo,
                                                    unsigned int khi) const;
template ADReal SplineInterpolationBase::sample<ADReal>(const std::vector<Real> & x,
                                                        const std::vector<Real> & y,
                                                        const std::vector<Real> & y2,
                                                        const ADReal & x_int,
                                                        unsigned int klo,
                                                        unsigned int khi) const;

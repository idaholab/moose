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

#include "SplineInterpolationBase.h"
#include "MooseError.h"

SplineInterpolationBase::SplineInterpolationBase()
{}

void
SplineInterpolationBase::spline(const std::vector<Real> & x, const std::vector<Real> & y, std::vector<Real> & y2, Real yp1/* = 1e30*/, Real ypn/* = 1e30*/)
{
  unsigned int n = x.size();
  if (n == 0)
    mooseError("Solving for an empty SplineInterpolation.");

  std::vector<Real> u(n, 0.);
  y2.resize(n, 0.);

  if (yp1 >= 1e30)
    y2[0] = u[0] = 0.;
  else
  {
    y2[0] = -0.5;
    u[0] = (3.0 / (x[1] - x[0])) * ((y[1] - y[0]) / (x[1] - x[0]) - yp1);
  }
  // decomposition of tri-diagonal algorithm (y2 and u are used for temporary storage)
  for (unsigned int i = 1; i + 1 < n; i++)
  {
    Real sig = (x[i] - x[i - 1]) / (x[i + 1] - x[i - 1]);
    Real p = sig * y2[i - 1] + 2.0;
    y2[i] = (sig - 1.0) / p;
    u[i] = (y[i + 1] - y[i]) / (x[i + 1] - x[i]) - (y[i] - y[i - 1]) / (x[i] - x[i - 1]);
    u[i] = (6.0 * u[i] / (x[i+1] - x[i - 1]) - sig * u[i - 1]) / p;
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
  for (int k = n - 2; k >= 0; k--)
    y2[k] = y2[k] * y2[k + 1] + u[k];
}

void
SplineInterpolationBase::findInterval(const std::vector<Real> & x, Real x_int, unsigned int & klo, unsigned int & khi) const
{
  klo = 0;
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

void
SplineInterpolationBase::computeCoeffs(const std::vector<Real> & x, unsigned int klo, unsigned int khi, Real x_int, Real & h, Real & a, Real & b) const
{
  h = x[khi] - x[klo];
  if (h == 0)
    mooseError("The values of x must be distinct");
  a = (x[khi] - x_int) / h;
  b = (x_int - x[klo]) / h;
}

Real
SplineInterpolationBase::sample(const std::vector<Real> & x, const std::vector<Real> & y, const std::vector<Real> & y2, Real x_int) const
{
  unsigned int klo, khi;
  findInterval(x, x_int, klo, khi);

  Real h, a, b;
  computeCoeffs(x, klo, khi, x_int, h, a, b);

  return a * y[klo] + b * y[khi] + ((a*a*a - a) * y2[klo] + (b*b*b - b) * y2[khi]) * (h*h) / 6.0;
}

Real
SplineInterpolationBase::sampleDerivative(const std::vector<Real> & x, const std::vector<Real> & y, const std::vector<Real> & y2, Real x_int) const
{
  unsigned int klo, khi;
  findInterval(x, x_int, klo, khi);

  Real h, a, b;
  computeCoeffs(x, klo, khi, x_int, h, a, b);

  return (y[khi] - y[klo]) / h - (((3.0 * a*a - 1.0) * y2[klo] + (3.0 * b*b - 1.0) * -y2[khi]) * h / 6.0);
}

Real
SplineInterpolationBase::sample2ndDerivative(const std::vector<Real> & x, const std::vector<Real> & y, const std::vector<Real> & y2, Real x_int) const
{
  unsigned int klo, khi;
  findInterval(x, x_int, klo, khi);

  Real h, a, b;
  computeCoeffs(x, klo, khi, x_int, h, a, b);

  return a * y2[klo] + b * y2[khi];
}

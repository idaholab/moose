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

#include "MonotoneCubicInterpolation.h"

#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <cassert>
#include <cmath>

MonotoneCubicInterpolation::MonotoneCubicInterpolation()
{}

MonotoneCubicInterpolation::MonotoneCubicInterpolation(const std::vector<double> & x, const std::vector<double> & y):
    _x(x),
    _y(y)
{
  errorCheck();
  solve();
}

void
MonotoneCubicInterpolation::setData(const std::vector<double> & x, const std::vector<double> & y)
{
  _x = x;
  _y = y;
  errorCheck();
  solve();
}

void
MonotoneCubicInterpolation::errorCheck()
{
  if (_x.size() != _y.size())
    throw std::domain_error("MonotoneCubicInterpolation: x and y vectors are not the same length");

  bool error = false;
  for (unsigned i = 0; !error && i + 1 < _x.size(); ++i)
    if (_x[i] >= _x[i+1])
      error = true;

  if (error)
    throw std::domain_error("x-values are not strictly increasing");

  checkMonotone();
  if (_monotonic_status == monotonic_not)
    throw std::domain_error("Don't ask for a monotonic interpolation routine if your dependent variable data isn't monotonic.");
}

double
MonotoneCubicInterpolation::sign(const double & x) const
{
  if (x < 0)
    return -1;
  else if (x > 0)
    return 1;
  else
    return 0;
}

void
MonotoneCubicInterpolation::checkMonotone()
{
  double y_diff = _y[1] - _y[0];
  double s = sign(y_diff);
  for (unsigned int i = 1; i < _y.size() - 1; ++i)
  {
    y_diff = _y[i+1] - _y[i];
    if (s == 0)
      s = sign(y_diff);
    if (s * y_diff < 0)
    {
      _monotonic_status = monotonic_not;
      return;
    }
  }
  if (s > 0)
    _monotonic_status = monotonic_increase;
  else if (s < 0)
    _monotonic_status = monotonic_decrease;
  else
    _monotonic_status = monotonic_constant;
}

double
MonotoneCubicInterpolation::phi(const double & t) const
{
  return 3. * t * t - 2. * t * t * t;
}

double
MonotoneCubicInterpolation::phiPrime(const double & t) const
{
  return 6. * t - 6. * t * t;
}

double
MonotoneCubicInterpolation::phiDoublePrime(const double & t) const
{
  return 6. - 12. * t;
}

double
MonotoneCubicInterpolation::psi(const double & t) const
{
  return t * t * t - t * t;
}

double
MonotoneCubicInterpolation::psiPrime(const double & t) const
{
  return 3. * t * t - 2. * t;
}

double
MonotoneCubicInterpolation::psiDoublePrime(const double & t) const
{
  return 6. * t - 2.;
}

double
MonotoneCubicInterpolation::h1(const double & xhi, const double & xlo, const double & x) const
{
  double h = xhi - xlo;
  double t = (xhi - x) / h;
  return phi(t);
}

double
MonotoneCubicInterpolation::h1Prime(const double & xhi, const double & xlo, const double & x) const
{
  double h = xhi - xlo;
  double t = (xhi - x) / h;
  double tPrime = -1. / h;
  return phiPrime(t) * tPrime;
}

double
MonotoneCubicInterpolation::h1DoublePrime(const double & xhi, const double & xlo, const double & x) const
{
  double h = xhi - xlo;
  double t = (xhi - x) / h;
  double tPrime = -1. / h;
  return phiDoublePrime(t) * tPrime * tPrime;
}

double
MonotoneCubicInterpolation::h2(const double & xhi, const double & xlo, const double & x) const
{
  double h = xhi - xlo;
  double t = (x - xlo) / h;
  return phi(t);
}

double
MonotoneCubicInterpolation::h2Prime(const double & xhi, const double & xlo, const double & x) const
{
  double h = xhi - xlo;
  double t = (x - xlo) / h;
  double tPrime = 1. / h;
  return phiPrime(t) * tPrime;
}

double
MonotoneCubicInterpolation::h2DoublePrime(const double & xhi, const double & xlo, const double & x) const
{
  double h = xhi - xlo;
  double t = (x - xlo) / h;
  double tPrime = 1. / h;
  return phiDoublePrime(t) * tPrime * tPrime;
}

double
MonotoneCubicInterpolation::h3(const double & xhi, const double & xlo, const double & x) const
{
  double h = xhi - xlo;
  double t = (xhi - x) / h;
  return -h * psi(t);
}

double
MonotoneCubicInterpolation::h3Prime(const double & xhi, const double & xlo, const double & x) const
{
  double h = xhi - xlo;
  double t = (xhi - x) / h;
  double tPrime = -1. / h;
  return -h * psiPrime(t) * tPrime; // psiPrime(t)
}

double
MonotoneCubicInterpolation::h3DoublePrime(const double & xhi, const double & xlo, const double & x) const
{
  double h = xhi - xlo;
  double t = (xhi - x) / h;
  double tPrime = -1. / h;
  return psiDoublePrime(t) * tPrime;
}

double
MonotoneCubicInterpolation::h4(const double & xhi, const double & xlo, const double & x) const
{
  double h = xhi - xlo;
  double t = (x - xlo) / h;
  return h * psi(t);
}

double
MonotoneCubicInterpolation::h4Prime(const double & xhi, const double & xlo, const double & x) const
{
  double h = xhi - xlo;
  double t = (x - xlo) / h;
  double tPrime = 1. / h;
  return h * psiPrime(t) * tPrime; // psiPrime(t)
}

double
MonotoneCubicInterpolation::h4DoublePrime(const double & xhi, const double & xlo, const double & x) const
{
  double h = xhi - xlo;
  double t = (x - xlo) / h;
  double tPrime = 1. / h;
  return psiDoublePrime(t) * tPrime;
}

double
MonotoneCubicInterpolation::p(const double & xhi, const double & xlo, const double & fhi, const double & flo,
                              const double & dhi, const double & dlo, const double & x) const
{
  return flo * h1(xhi, xlo, x) + fhi * h2(xhi, xlo, x)
    + dlo * h3(xhi, xlo, x) + dhi * h4(xhi, xlo, x);
}

double
MonotoneCubicInterpolation::pPrime(const double & xhi, const double & xlo, const double & fhi, const double & flo,
                                   const double & dhi, const double & dlo, const double & x) const
{
  return flo * h1Prime(xhi, xlo, x) + fhi * h2Prime(xhi, xlo, x)
    + dlo * h3Prime(xhi, xlo, x) + dhi * h4Prime(xhi, xlo, x);
}

double
MonotoneCubicInterpolation::pDoublePrime(const double & xhi, const double & xlo, const double & fhi, const double & flo,
                                         const double & dhi, const double & dlo, const double & x) const
{
  return flo * h1DoublePrime(xhi, xlo, x) + fhi * h2DoublePrime(xhi, xlo, x)
    + dlo * h3DoublePrime(xhi, xlo, x) + dhi * h4DoublePrime(xhi, xlo, x);
}

void
MonotoneCubicInterpolation::initialize_derivs()
{
  for (unsigned int i = 1; i < _n_knots - 1; ++i)
    _yp[i] = (std::pow(_h[i-1], 2) * _y[i+1] - std::pow(_h[i], 2) * _y[i-1] - _y[i] * (_h[i-1] - _h[i]) * (_h[i-1] + _h[i])) / (_h[i-1] * _h[i] * (_h[i-1] * _h[i]));

  _yp[0] = (-std::pow(_h[0], 2) * _y[2] - _h[1] * _y[0] * (2*_h[0] + _h[1]) + _y[1] * std::pow(_h[0] + _h[1], 2)) / (_h[0] * _h[1] * (_h[0] + _h[1]));

  double hlast = _h[_n_intervals - 1];
  double hsecond = _h[_n_intervals - 2];
  double ylast = _y[_n_knots - 1];
  double ysecond = _y[_n_knots - 2];
  double ythird = _y[_n_knots - 3];
  _yp[_n_knots - 1] = (hsecond * ylast * (hsecond + 2 * hlast) + std::pow(hlast, 2) * ythird - ysecond * std::pow(hsecond + hlast, 2)) / (hsecond * hlast * (hsecond + hlast));
}

void
MonotoneCubicInterpolation::modify_derivs(const double & alpha, const double & beta, const double & delta, double & yp_lo, double & yp_hi)
{
  double tau = 3. / std::sqrt(std::pow(alpha, 2) + std::pow(beta, 2));
  double alpha_star = alpha * tau;
  double beta_star = beta * tau;
  yp_lo = alpha_star * delta;
  yp_hi = beta_star * delta;
}

void
MonotoneCubicInterpolation::solve()
{
  _n_knots = _x.size(), _n_intervals = _x.size() - 1;
  _h.resize(_n_intervals);
  _yp.resize(_n_knots);
  _delta.resize(_n_intervals);
  _alpha.resize(_n_intervals);
  _beta.resize(_n_intervals);

  for (unsigned int i = 0; i < _n_intervals; ++i)
    _h[i] = _x[i+1] - _x[i];

  initialize_derivs();
  for (unsigned int i = 0; i < _n_intervals; ++i)
    _delta[i] = (_y[i+1] - _y[i]) / _h[i];
  if (sign(_delta[0]) != sign(_yp[0]))
    _yp[0] = 0;
  if (sign(_delta[_n_intervals - 1]) != sign(_yp[_n_knots - 1]))
    _yp[_n_knots - 1] = 0;

  for (unsigned int i = 0; i < _n_intervals; ++i)
  {
    // Test for zero slope
    if (_yp[i] == 0 && _delta[i] == 0)
      _alpha[i] = 1;
    else if (_delta[i] == 0)
      _alpha[i] = 4;
    else
      _alpha[i] = _yp[i] / _delta[i];

    // Test for zero slope
    if (_yp[i+1] == 0 && _delta[i] == 0)
      _beta[i] = 1;
    else if (_delta[i] == 0)
      _beta[i] = 4;
    else
      _beta[i] = _yp[i+1] / _delta[i];

    // check if outside region of monotonicity
    if (std::pow(_alpha[i], 2) + std::pow(_beta[i], 2) > 9.)
      modify_derivs(_alpha[i], _beta[i], _delta[i], _yp[i], _yp[i+1]);
  }
}

void
MonotoneCubicInterpolation::findInterval(const double & x, unsigned int & klo, unsigned int & khi) const
{
  klo = 0;
  khi = _n_knots - 1;
  while (khi - klo > 1)
  {
    unsigned int k = (khi + klo) >> 1;
    if (_x[k] > x)
      khi = k;
    else
      klo = k;
  }
}

double
MonotoneCubicInterpolation::sample(const double & x) const
{
  // sanity check (empty MontoneCubicInterpolations are constructable
  // so we cannot put this into the errorCheck)
  assert(_x.size() > 0);

  unsigned int klo, khi;
  findInterval(x, klo, khi);
  return p(_x[khi], _x[klo], _y[khi], _y[klo], _yp[khi], _yp[klo], x);
}

double
MonotoneCubicInterpolation::sampleDerivative(const double & x) const
{
  unsigned int klo, khi;
  findInterval(x, klo, khi);
  return pPrime(_x[khi], _x[klo], _y[khi], _y[klo], _yp[khi], _yp[klo], x);
}

double
MonotoneCubicInterpolation::sample2ndDerivative(const double & x) const
{
  unsigned int klo, khi;
  findInterval(x, klo, khi);
  return pDoublePrime(_x[khi], _x[klo], _y[khi], _y[klo], _yp[khi], _yp[klo], x);
}

void
MonotoneCubicInterpolation::dumpCSV(std::string filename, const std::vector<double> & xnew)
{
  unsigned int n = xnew.size();
  std::vector<double> ynew(n), ypnew(n), yppnew(n);

  std::ofstream out(filename.c_str());
  for (unsigned int i = 0; i < n; ++i)
  {
    ynew[i] = sample(xnew[i]);
    ypnew[i] = sampleDerivative(xnew[i]);
    yppnew[i] = sample2ndDerivative(xnew[i]);
    out << xnew[i] << ", " << ynew[i] << ", " << ypnew[i] << ", " << yppnew[i] << "\n";
  }
  out.close();
}

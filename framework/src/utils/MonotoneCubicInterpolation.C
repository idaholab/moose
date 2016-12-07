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
#include "MooseError.h"

MonotoneCubicInterpolation::MonotoneCubicInterpolation()
{}

MonotoneCubicInterpolation::MonotoneCubicInterpolation(const std::vector<Real> x, const std::vector<Real> y):
    _x(x),
    _y(y)
{
  errorCheck();
  solve();
}

void
MonotoneCubicInterpolation::errorCheck()
{
  if (_x.size() != _y.size())
    mooseError("MonotoneCubicInterpolation: x and y vectors are not the same length");

  bool error = false;
  for (unsigned i = 0; !error && i + 1 < _x.size(); ++i)
    if (_x[i] >= _x[i+1])
      error = true;

  if (error)
    mooseError("x-values are not strictly increasing");

  checkMonotone();
  if (_monotonic_e == "monotonic_not")
    mooseError("Don't ask for a monotonic interpolation routine if your dependent variable data isn't monotonic.");
}

Real
MonotoneCubicInterpolation::sign(Real x) const
{
  return x < 0 ? -1. : (x > 0 ? 1. : 0.);
}

void
MonotoneCubicInterpolation::checkMonotone()
{
  Real y_diff = _y[1] - _y[0];
  Real s = sign(y_diff);
  for (unsigned int i = 1; i < _y.size(); ++i)
  {
    y_diff = _y[i+1] - _y[i];
    if (s == 0)
      s = sign(y_diff);
    if (s * y_diff < 0)
    {
      _monotonic_e = "monotonic_not";
      return;
    }
  }
  s > 0 ? _monotonic_e = "monotonic_increase" :
    (s < 0 ? _monotonic_e = "monotonic_decrease" :
     _monotonic_e = "montonic_constant");
}

Real
MonotoneCubicInterpolation::phi(const Real & t) const
{
  return 3. * std::pow(t, 2) - 2. * std::pow(t, 3);
}

Real
MonotoneCubicInterpolation::psi(const Real & t) const
{
  return std::pow(t, 3) - std::pow(t, 2);
}

Real
MonotoneCubicInterpolation::H1(const Real & xhi, const Real & xlo, const Real & x) const
{
  Real h = xhi - xlo;
  Real t = (xhi - x) / h;
  return phi(t);
}

Real
MonotoneCubicInterpolation::H2(const Real & xhi, const Real & xlo, const Real & x) const
{
  Real h = xhi - xlo;
  Real t = (x - xlo) / h;
  return phi(t);
}

Real
MonotoneCubicInterpolation::H3(const Real & xhi, const Real & xlo, const Real & x) const
{
  Real h = xhi - xlo;
  Real t = (xhi - x) / h;
  return -h * psi(t);
}

Real
MonotoneCubicInterpolation::H4(const Real & xhi, const Real & xlo, const Real & x) const
{
  Real h = xhi - xlo;
  Real t = (x - xlo) / h;
  return h * psi(t);
}

Real
MonotoneCubicInterpolation::p(const Real & xhi, const Real & xlo, const Real & fhi, const Real & flo,
                                const Real & dhi, const Real & dlo, const Real & x) const
{
  return flo * H1(xhi, xlo, x) + fhi * H2(xhi, xlo, x)
    + dlo * H3(xhi, xlo, x) + dhi * H4(xhi, xlo, x);
}

void
MonotoneCubicInterpolation::initialize_derivs()
{
  for (unsigned int i = 1; i < _n_knots - 1; ++i)
    _yp[i] = (std::pow(_h[i-1], 2) * _y[i+1] - std::pow(_h[i], 2) * _y[i-1] - _y[i] * (_h[i-1] - _h[i]) * (_h[i-1] + _h[i])) / (_h[i-1] * _h[i] * (_h[i-1] * _h[i]));
  
  _yp[0] = (-std::pow(_h[0], 2) * _y[2] - _h[1] * _y[0] * (2*_h[0] + _h[1]) + _y[1] * std::pow(_h[0] + _h[1], 2)) / (_h[0] * _h[1] * (_h[0] + _h[1]));

  Real hlast = _h[_n_intervals - 1];
  Real hsecond = _h[_n_intervals - 2];
  Real ylast = _y[_n_knots - 1];
  Real ysecond = _y[_n_knots - 2];
  Real ythird = _y[_n_knots - 3];
  _yp[_n_knots - 1] = (hsecond * ylast * (hsecond + 2 * hlast) + std::pow(hlast, 2) * ythird - ysecond * std::pow(hsecond + hlast, 2)) / (hsecond * hlast * (hsecond + hlast));
}

void
MonotoneCubicInterpolation::modify_derivs(const Real & alpha, const Real & beta, const Real & delta, Real & yp_lo, Real & yp_hi)
{
  Real tau = 3. / std::sqrt(std::pow(alpha, 2) + std::pow(beta, 2));
  Real alpha_star = alpha * tau;
  Real beta_star = beta * tau;
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

  initialize_derivs();

  for (unsigned int i = 0; i < _n_intervals; ++i)
  {
    _delta[i] = (_y[i+1] - _y[i]) / _h[i];

    // Test for zero slope
    if (_yp[i] == _delta[i] == 0)
      _alpha[i] = 1;
    else if (_delta[i] == 0)
      _alpha[i] = 4;
    else
      _alpha[i] = _yp[i] / _delta[i];

    // Test for zero slope
    if (_yp[i+1] == _delta[i] == 0)
      _beta[i] = 1;
    else if (_delta[i] == 0)
      _beta[i] = 4;
    else
      _beta[i] = _yp[i+1] / _delta[i];

    // check if outside region of monotonicity
    if (std::pow(_alpha[i], 2) + std::pow(_beta[i], 2) > 3.)
      modify_derivs(_alpha[i], _beta[i], _delta[i], _yp[i], _yp[i+1]);
  }
}

Real
MonotoneCubicInterpolation::sample(Real x) const
{
  for (unsigned int i = 0; i < _n_intervals; ++i)
    if (_x[i] <= x <= _x[i+1])
      return p(_x[i+1], _x[i], _y[i+1], _y[i], _yp[i+1], _yp[i], x);

  mooseError("x not in interpolation range");
  return 0;
}
        

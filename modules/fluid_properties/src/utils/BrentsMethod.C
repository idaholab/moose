//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BrentsMethod.h"
#include "MooseError.h"
#include "Conversion.h"

namespace BrentsMethod
{
void
bracket(std::function<Real(Real)> const & f, Real & x1, Real & x2)
{
  Real f1, f2;
  // Factor to scale the interval by
  Real factor = 1.6;
  // Maximum number of times to attempt bracketing the interval
  unsigned int n = 50;
  // Small positive value to keep guess above
  Real eps = 1.0e-10;

  // If the initial guesses are identical
  if (x1 == x2)
    throw MooseException("Bad initial range (0) used in BrentsMethod::bracket");

  f1 = f(x1);
  f2 = f(x2);

  if (f1 * f2 > 0.0)
  {
    unsigned int iter = 0;
    while (f1 * f2 > 0.0)
    {
      if (std::abs(f1) < std::abs(f2))
      {
        x1 += factor * (x1 - x2);
        x1 = (x1 < eps ? eps : x1);
        f1 = f(x1);
      }
      else
      {
        x2 += factor * (x2 - x1);
        x2 = (x2 < eps ? eps : x2);
        f2 = f(x2);
      }
      /// Increment counter
      iter++;
      if (iter >= n)
        throw MooseException("No bracketing interval found by BrentsMethod::bracket after " +
                             Moose::stringify(n) + " iterations");
    }
  }
}

Real
root(std::function<Real(Real)> const & f, Real x1, Real x2, Real tol)
{
  Real a = x1, b = x2, c = x2, d = 0.0, e = 0.0, min1, min2;
  Real fa = f(a);
  Real fb = f(b);
  Real fc, p, q, r, s, tol1, xm;
  unsigned int iter_max = 100;
  Real eps = 1.0e-12;

  if (fa * fb > 0.0)
    throw MooseException("Root must be bracketed in BrentsMethod::root");

  fc = fb;
  for (unsigned int i = 1; i <= iter_max; ++i)
  {
    if (fb * fc > 0.0)
    {
      // Rename a,b and c and adjust bounding interval d
      c = a;
      fc = fa;
      d = b - a;
      e = d;
    }
    if (std::abs(fc) < std::abs(fb))
    {
      a = b;
      b = c;
      c = a;
      fa = fb;
      fb = fc;
      fc = fa;
    }
    // Convergence check tolerance
    tol1 = 2.0 * eps * std::abs(b) + 0.5 * tol;
    xm = 0.5 * (c - b);

    if (std::abs(xm) <= tol1 || fb == 0.0)
      return b;

    if (std::abs(e) >= tol1 && std::abs(fa) > std::abs(fb))
    {
      // Attempt inverse quadratic interpolation
      s = fb / fa;
      if (a == c)
      {
        p = 2.0 * xm * s;
        q = 1.0 - s;
      }
      else
      {
        q = fa / fc;
        r = fb / fc;
        p = s * (2.0 * xm * q * (q - r) - (b - a) * (r - 1.0));
        q = (q - 1.0) * (r - 1.0) * (s - 1.0);
      }
      // Check whether in bounds
      if (p > 0.0)
        q = -q;
      p = std::abs(p);
      min1 = 3.0 * xm * q - std::abs(tol1 * q);
      min2 = std::abs(e * q);

      if (2.0 * p < (min1 < min2 ? min1 : min2))
      {
        // Accept interpolation
        e = d;
        d = p / q;
      }
      else
      {
        // Interpolation failed, use bisection
        d = xm;
        e = d;
      }
    }
    else
    {
      // Bounds decreasing too slowly, use bisection
      d = xm;
      e = d;
    }
    // Move last best guess to a
    a = b;
    fa = fb;
    // Evaluate new trial root
    if (std::abs(d) > tol1)
      b += d;
    else
    {
      Real sgn = (xm >= 0.0 ? std::abs(tol1) : -std::abs(tol1));
      b += sgn;
    }

    fb = f(b);
  }

  throw MooseException("Maximum number of iterations exceeded in BrentsMethod::root");
  return 0.0; // Should never get here
}
} // namespace BrentsMethod

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "FunctionTestUtils.h"
#include "MooseUtils.h"

namespace FunctionTestUtils
{

void
testTimeIntegral(
    const Function & f, Real t1, Real t2, const Point & p, unsigned int n_intervals, Real tol)
{
  // numerically integrate function using trapezoidal rule
  Real integral_approx = 0;
  const Real dt = (t2 - t1) / n_intervals;
  Real tA = t1;
  Real fA = f.value(tA, p);
  Real tB, fB;
  for (const auto i : make_range(n_intervals))
  {
    // to avoid unused variable compiler warning
    std::ignore = i;

    tB = tA + dt;
    fB = f.value(tB, p);

    integral_approx += 0.5 * (fA + fB) * dt;

    tA = tB;
    fA = fB;
  }

  const Real integral = f.timeIntegral(t1, t2, p);

  Real err;
  if (MooseUtils::absoluteFuzzyEqual(integral_approx, 0.0))
    // absolute error test: interpret tol as absolute tolerance
    err = std::abs(integral - integral_approx);
  else
    // relative error test: interpret tol as relative tolerance
    err = std::abs((integral - integral_approx) / integral_approx);

  EXPECT_LE(err, tol);
}

}

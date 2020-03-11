//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "gtest/gtest.h"
#include <vector>
#include <cmath>
#include "PolynomialQuadrature.h"
#include "libmesh/utility.h"

using namespace PolynomialQuadrature;

TEST(StochasticTools, PolynomialQuadrature)
{
  Real tol = 1e-14;

  {
    Real lower_bound = 0.7;
    Real upper_bound = 17.3;
    Real x = 7.78254;
    Real xref = 2 / (upper_bound - lower_bound) * (x - (upper_bound + lower_bound) / 2);

    Real poly_val = legendre(5, x, lower_bound, upper_bound);
    Real val = 1 / 8.0 * (63.0 * Utility::pow<5>(xref) - 70.0 * xref * xref * xref + 15.0 * xref);
    EXPECT_NEAR(poly_val, val, tol);

    std::vector<Real> xq_ref(5);
    xq_ref[0] = -1 / 3.0 * std::sqrt(5.0 + 2.0 * std::sqrt(10.0 / 7.0));
    xq_ref[1] = -1 / 3.0 * std::sqrt(5.0 - 2.0 * std::sqrt(10.0 / 7.0));
    xq_ref[2] = 0.0;
    xq_ref[3] = -xq_ref[0];
    xq_ref[4] = -xq_ref[1];
    for (unsigned int n = 0; n < 5; ++n)
      xq_ref[n] = xq_ref[n] * (upper_bound - lower_bound) / 2.0 + (upper_bound + lower_bound) / 2.0;

    std::vector<Real> wq_ref(5);
    wq_ref[0] = (322.0 - 13.0 * std::sqrt(70.0)) / 1800.0;
    wq_ref[1] = (322.0 + 13.0 * std::sqrt(70.0)) / 1800.0;
    wq_ref[2] = 64.0 / 225.0;
    wq_ref[3] = wq_ref[0];
    wq_ref[4] = wq_ref[1];

    std::vector<Real> xq;
    std::vector<Real> wq;
    gauss_legendre(4, xq, wq, lower_bound, upper_bound);
    for (unsigned int n = 0; n < 5; ++n)
    {
      EXPECT_NEAR(xq[n], xq_ref[n], tol);
      EXPECT_NEAR(wq[n], wq_ref[n], tol);
    }
  }

  {
    Real mu = 4.7518;
    Real sig = 0.9438;
    Real x = 3.9587;
    Real xref = (x - mu) / sig;

    Real poly_val = hermite(4, x, mu, sig);
    Real val = Utility::pow<4>(xref) - 6.0 * xref * xref + 3.0;
    EXPECT_NEAR(poly_val, val, tol);

    std::vector<Real> xq_ref(4);
    xq_ref[0] = -std::sqrt(3.0 + std::sqrt(6.0));
    xq_ref[1] = -std::sqrt(3.0 - std::sqrt(6.0));
    xq_ref[2] = -xq_ref[1];
    xq_ref[3] = -xq_ref[0];
    for (unsigned int n = 0; n < 4; ++n)
      xq_ref[n] = xq_ref[n] * sig + mu;

    std::vector<Real> wq_ref(4);
    wq_ref[0] = 1.0 / (4.0 * (3.0 + std::sqrt(6.0)));
    wq_ref[1] = 1.0 / (4.0 * (3.0 - std::sqrt(6.0)));
    wq_ref[2] = wq_ref[1];
    wq_ref[3] = wq_ref[0];

    std::vector<Real> xq;
    std::vector<Real> wq;
    gauss_hermite(3, xq, wq, mu, sig);
    for (unsigned int n = 0; n < 4; ++n)
    {
      EXPECT_NEAR(xq[n], xq_ref[n], tol);
      EXPECT_NEAR(wq[n], wq_ref[n], tol);
    }
  }
}

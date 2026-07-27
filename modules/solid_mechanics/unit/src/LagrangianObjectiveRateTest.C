//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "LagrangianObjectiveRate.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

// Unit tests for the objective-stress-rate free functions. These exercise the rate math in
// isolation -- no host material, no assembly -- which is exactly what the free-function form
// (vs the former host-coupled strategy hierarchy) enables.

using namespace LagrangianObjectiveRates;

namespace
{
/// Isotropic small-strain tangent (minor- and major-symmetric).
RankFourTensor
isotropicC()
{
  RankFourTensor C;
  C.fillFromInputVector({100.0, 60.0}, RankFourTensor::symmetric_isotropic); // {lambda, mu}
  return C;
}

/// A representative non-trivial cumulative Cauchy stress at step n (xx, yy, zz, yz, xz, xy).
const RankTwoTensor kSigmaOld(30.0, -10.0, 15.0, 4.0, -3.0, 6.0);

/// Build a self-consistent Inputs bundle for a given spatial velocity gradient increment `dL`:
///   dS  = C : sym(dL)        (elastic small-stress increment)
///   dW  = skew(dL)
///   d(dL)/dF = I^(4),  d(dW)/dF = skew projector  (so the returned tangent is exactly d(sigma)/d(dL))
Inputs
makeInputs(const RankFourTensor & C, const RankTwoTensor & dL)
{
  Inputs in;
  in.small_jacobian = C;
  in.cauchy_stress_old = kSigmaOld;
  in.dL = dL;
  in.dW = 0.5 * (dL - dL.transpose());
  in.dS = C * (0.5 * (dL + dL.transpose()));
  in.d_dL_d_F = RankFourTensor::IdentityFour();
  // d(skew(A))/dA = 1/2 (I^(4) - swap_ij), swap_ij = I^(4) with its first index pair transposed.
  in.d_dW_d_F =
      0.5 * (RankFourTensor::IdentityFour() - RankFourTensor::IdentityFour().transposeIj());
  return in;
}

/// Central-difference the returned `cauchy_jacobian` against d(cauchy_stress)/d(dL).
void
checkJacobianFD(Outputs (*rate)(const Inputs &, bool),
                const RankFourTensor & C,
                const RankTwoTensor & dL)
{
  const RankFourTensor J = rate(makeInputs(C, dL), true).cauchy_jacobian;
  const Real eps = 1.0e-6;
  RankFourTensor Jfd;
  for (unsigned int k = 0; k < 3; ++k)
    for (unsigned int l = 0; l < 3; ++l)
    {
      RankTwoTensor dLp = dL, dLm = dL;
      dLp(k, l) += eps;
      dLm(k, l) -= eps;
      const RankTwoTensor d = (rate(makeInputs(C, dLp), false).cauchy_stress -
                               rate(makeInputs(C, dLm), false).cauchy_stress) /
                              (2.0 * eps);
      for (unsigned int i = 0; i < 3; ++i)
        for (unsigned int j = 0; j < 3; ++j)
          Jfd(i, j, k, l) = d(i, j);
    }
  EXPECT_LT((Jfd - J).L2norm(), 1.0e-4 * J.L2norm());
}
}

// With no kinematic increment every rate leaves the stress unadvected: sigma_{n+1} = sigma_n + dS.
TEST(LagrangianObjectiveRateTest, zeroIncrement)
{
  Inputs in = makeInputs(isotropicC(), RankTwoTensor()); // dL = 0 -> dS = 0, dW = 0
  in.rotation = RankTwoTensor::Identity();
  in.rotation_old = RankTwoTensor::Identity();
  in.inv_df = RankTwoTensor::Identity();
  in.inv_def_grad = RankTwoTensor::Identity();
  const RankTwoTensor expected = in.cauchy_stress_old + in.dS;
  EXPECT_LT((truesdell(in, false).cauchy_stress - expected).L2norm(), 1.0e-12);
  EXPECT_LT((jaumann(in, false).cauchy_stress - expected).L2norm(), 1.0e-12);
  EXPECT_LT((greenNaghdi(in, false).cauchy_stress - expected).L2norm(), 1.0e-12);
  EXPECT_LT((rashid(in, false).cauchy_stress - expected).L2norm(), 1.0e-12);
}

// Consistent-tangent checks: the returned cauchy_jacobian must equal d(cauchy_stress)/d(dL).
TEST(LagrangianObjectiveRateTest, truesdellJacobian)
{
  checkJacobianFD(&truesdell, isotropicC(), RankTwoTensor(0.03, -0.02, 0.01, 0.015, -0.01, 0.02));
}

TEST(LagrangianObjectiveRateTest, jaumannJacobian)
{
  checkJacobianFD(&jaumann, isotropicC(), RankTwoTensor(0.03, -0.02, 0.01, 0.015, -0.01, 0.02));
}

TEST(LagrangianObjectiveRateTest, rashidJacobian)
{
  // Add an antisymmetric part so r_hat = exp(skew(dL)) != I and the Rodrigues chain is exercised.
  RankTwoTensor dL(0.03, -0.02, 0.01, 0.015, -0.01, 0.02);
  dL(0, 1) += 0.02;
  dL(1, 0) -= 0.02;
  checkJacobianFD(&rashid, isotropicC(), dL);
}

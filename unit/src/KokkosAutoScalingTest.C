//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_KOKKOS_ENABLED

#include "gtest_include.h"

#include "Executioner.h"
#include "FEProblemBase.h"
#include "MooseAppTestUtils.h"
#include "MooseMain.h"
#include "MooseVariableBase.h"

#include <utility>

using MooseAppTestUtils::Args;

namespace
{
Real
computeScalingFactor(const bool include_off_diagonals)
{
  // mimic ConsoleExecuteOnTest.C that runs a MOOSE app with the given input file and returns the
  // scaling factor of the variable
  Args args({"-i",
             "files/KokkosAutoScalingTest/scaling.i",
             "Executioner/off_diagonals_in_auto_scaling=" +
                 std::string(include_off_diagonals ? "true" : "false")});
  const auto app = Moose::createMooseApp("MooseUnitApp", args.argc(), args.argv());
  app->run();
  return app->getExecutioner()->feProblem().getVariable(0, "u").scalingFactor();
}

std::pair<Real, Real>
computeTwoVariableScalingFactors(const bool group_variables)
{
  std::vector<std::string> arg_list = {"-i",
                                       "files/KokkosAutoScalingTest/scaling.i",
                                       "Executioner/off_diagonals_in_auto_scaling=false",
                                       "Variables/v/family=LAGRANGE",
                                       "NodalKernels/reaction_v/type=KokkosReactionNodalKernel",
                                       "NodalKernels/reaction_v/variable=v",
                                       "NodalKernels/reaction_v/coeff=12"};

  if (group_variables)
    arg_list.push_back("Executioner/scaling_group_variables=u v");

  Args args(arg_list);
  const auto app = Moose::createMooseApp("MooseUnitApp", args.argc(), args.argv());
  app->run();

  const auto & fe_problem = app->getExecutioner()->feProblem();
  return {fe_problem.getVariable(0, "u").scalingFactor(),
          fe_problem.getVariable(0, "v").scalingFactor()};
}
}

TEST(KokkosAutoScaling, Diagonal)
{
  // Each linear element contributes (1 / h) * [1 -1; -1 1]. With three elements
  // on [0, 1], h = 1 / 3 and the assembled matrix is
  //
  //   (1 / h) *   [ 1 -1  0  0]
  //               [-1  2 -1  0]
  //               [ 0 -1  2 -1]
  //               [ 0  0 -1  1].
  //
  // The largest diagonal is therefore 2 / h = 6.
  EXPECT_NEAR(computeScalingFactor(false), 1.0 / 6.0, 1e-12);
}

TEST(KokkosAutoScaling, AbsoluteRowSum)
{
  // An interior row is (1 / h) * [-1 2 -1], whose absolute row sum is
  // (1 + 2 + 1) / h = 4 / h = 12.
  EXPECT_NEAR(computeScalingFactor(true), 1.0 / 12.0, 1e-12);
}

TEST(KokkosAutoScaling, SeparateVariables)
{
  // For u, the diffusion kernel gives
  //
  //        [ 3 -3  0  0]
  //        [-3  6 -3  0]
  //   Ju = [ 0 -3  6 -3].
  //        [ 0  0 -3  3]
  //
  // The maximum diagonal is therefore 6.
  //
  // For v, KokkosReactionNodalKernel with coeff = 12 contributes directly
  // to the diagonal at each node:
  //
  //        [12  0  0  0]
  //        [ 0 12  0  0]
  //   Jv = [ 0  0 12  0].
  //        [ 0  0  0 12]
  //
  // The maximum diagonal is therefore 12.
  const auto [u_factor, v_factor] = computeTwoVariableScalingFactors(false);

  EXPECT_NEAR(u_factor, 1.0 / 6.0, 1e-12);
  EXPECT_NEAR(v_factor, 1.0 / 12.0, 1e-12);
}

TEST(KokkosAutoScaling, GroupVariables)
{
  // Since u and v are uncoupled, the two-variable Jacobian has the block form
  //
  //       [Ju  0]
  //   J = [     ],
  //       [ 0 Jv]
  //
  // where
  //
  //        [ 3 -3  0  0]               [12  0  0  0]
  //        [-3  6 -3  0]               [ 0 12  0  0]
  //   Ju = [ 0 -3  6 -3],         Jv = [ 0  0 12  0].
  //        [ 0  0 -3  3]               [ 0  0  0 12]
  //
  // Therefore max(diag(Ju)) = 6 and max(diag(Jv)) = 12.
  // When u and v are in the same scaling group, both variables use
  // the largest diagonal value in the group, max(6, 12) = 12.
  const auto [u_factor, v_factor] = computeTwoVariableScalingFactors(true);

  EXPECT_NEAR(u_factor, 1.0 / 12.0, 1e-12);
  EXPECT_NEAR(v_factor, 1.0 / 12.0, 1e-12);
}

#endif // MOOSE_KOKKOS_ENABLED

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

#endif // MOOSE_KOKKOS_ENABLED

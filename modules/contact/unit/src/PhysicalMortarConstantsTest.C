//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "AppFactory.h"
#include "Executioner.h"
#include "FEProblemBase.h"
#include "MooseConfig.h"
#include "MooseMain.h"
#include "MortarTestUtils.h"

#include <limits>

// 3-D mortar contact requires enough AD derivative slots for the coupled
// displacement and LM DOFs on each element.
#if MOOSE_AD_MAX_DOFS_PER_ELEM >= 250

// Run the given input file with additional CLI args and return the cumulative
// nonlinear iteration count accumulated by the "cumulative" postprocessor, or infinity if a
// solve did not converge.
static Real
runCumulativeNL(const std::string & input,
                const std::vector<std::string> & extra_args,
                Real * contact_dofs = nullptr)
{
  // Build an argv suitable for Moose::createMooseApp
  std::vector<std::string> str_args = {"contact-unit", "-i", inputPath(__FILE__, input)};
  str_args.insert(str_args.end(), extra_args.begin(), extra_args.end());

  std::vector<char *> argv_vec;
  for (auto & s : str_args)
    argv_vec.push_back(s.data());
  int argc = static_cast<int>(argv_vec.size());

  auto app = Moose::createMooseApp("ContactApp", argc, argv_vec.data());
  app->run();
  auto & executioner = *app->getExecutioner();
  auto & problem = executioner.feProblem();
  if (contact_dofs)
    *contact_dofs = problem.getPostprocessorValueByName("contact");
  if (!executioner.lastSolveConverged())
    return std::numeric_limits<Real>::infinity();
  return problem.getPostprocessorValueByName("cumulative");
}

// Verify that c_strategy=physical (this fixture's default) reduces cumulative nonlinear
// iterations compared to the fixed c_normal=1e6 for frictionless mortar contact.
TEST(PhysicalMortarConstants, FrictionlessBeatsDefault)
{
  Real contact_dofs = 0;
  const Real physical_its = runCumulativeNL("frictionless_physical.i", {}, &contact_dofs);
  EXPECT_GT(contact_dofs, 0) << "no contact DOFs active - test is not exercising contact";
  const Real default_its =
      runCumulativeNL("frictionless_physical.i", {"Contact/mortar/c_strategy=user"});
  EXPECT_LT(physical_its, default_its)
      << "physical c_strategy (" << physical_its
      << " cumulative NL iters) should beat default c_normal=1e6 (" << default_its << " iters)";
}

// Verify that c_strategy=physical (this fixture's default) reduces cumulative nonlinear iterations
// compared to fixed c_normal=1e6/c_tangential=1e3 for frictional (Coulomb) mortar contact with
// sliding.
TEST(PhysicalMortarConstants, FrictionalBeatsDefault)
{
  Real contact_dofs = 0;
  const Real physical_its = runCumulativeNL("frictional_physical.i", {}, &contact_dofs);
  EXPECT_GT(contact_dofs, 0) << "no contact DOFs active - test is not exercising contact";
  const Real default_its = runCumulativeNL("frictional_physical.i",
                                           {"Contact/mortar/c_strategy=user",
                                            "Contact/mortar/c_tangential=1e3",
                                            "Executioner/abort_on_solve_fail=false"});
  EXPECT_LT(physical_its, default_its)
      << "physical c_strategy (" << physical_its
      << " cumulative NL iters) should beat defaults c_normal=1e6/c_tangential=1e3 (" << default_its
      << " iters)";
}

// Verify that each contact constant the physical strategy sets is rejected when also set by the
// user, so user-set and physically derived constants cannot be mixed.
TEST(PhysicalMortarConstants, UserConstantWithPhysicalThrows)
{
  for (const char * param : {"c_normal=1e6", "c_tangential=1e3", "normalize_c=true"})
    EXPECT_THROW(runCumulativeNL("frictional_physical.i", {"Contact/mortar/" + param}),
                 std::exception)
        << param;
}

// Verify that c_strategy=physical (this fixture's default) combined with automatic scaling is an
// error, since automatic scaling would rescale the already physically scaled Lagrange multiplier
// equations a second time.
TEST(PhysicalMortarConstants, PhysicalWithAutomaticScalingThrows)
{
  try
  {
    runCumulativeNL("frictionless_physical.i", {"Executioner/automatic_scaling=true"});
    FAIL() << "missing expected exception";
  }
  catch (const std::exception & e)
  {
    const std::string msg(e.what());
    ASSERT_TRUE(msg.find("'c_strategy = physical' cannot be used with automatic scaling") !=
                std::string::npos)
        << "failed with unexpected error: " << msg;
  }
}

#endif // MOOSE_AD_MAX_DOFS_PER_ELEM >= 250

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

// 3-D mortar contact requires enough AD derivative slots for the coupled
// displacement and LM DOFs on each element.
#if MOOSE_AD_MAX_DOFS_PER_ELEM >= 250

// Compute the absolute path to an input file stored alongside this source file in
// ../inputs/ by navigating from __FILE__ (the absolute path to this .C file).
static std::string
inputPath(const std::string & name)
{
  const std::string src = __FILE__;
  const auto pos = src.rfind("/src/");
  return src.substr(0, pos + 1) + "inputs/" + name;
}

// Run the given input file with additional CLI args and return the cumulative
// nonlinear iteration count accumulated by the "cumulative" postprocessor.
static Real
runCumulativeNL(const std::string & input,
                const std::vector<std::string> & extra_args,
                Real * contact_dofs = nullptr)
{
  // Build an argv suitable for Moose::createMooseApp
  std::vector<std::string> str_args = {"contact-unit", "-i", inputPath(input)};
  str_args.insert(str_args.end(), extra_args.begin(), extra_args.end());

  std::vector<char *> argv_vec;
  for (auto & s : str_args)
    argv_vec.push_back(s.data());
  int argc = static_cast<int>(argv_vec.size());

  auto app = Moose::createMooseApp("ContactApp", argc, argv_vec.data());
  app->run();
  auto & problem = app->getExecutioner()->feProblem();
  if (contact_dofs)
    *contact_dofs = problem.getPostprocessorValueByName("contact");
  return problem.getPostprocessorValueByName("cumulative");
}

// Verify that c_normal_strategy=physical reduces cumulative nonlinear iterations
// compared to the default c_normal=1e6 for frictionless mortar contact.
TEST(PhysicalMortarConstants, FrictionlessBeatsDefault)
{
  Real contact_dofs = 0;
  const Real default_its = runCumulativeNL("frictionless_physical.i", {}, &contact_dofs);
  EXPECT_GT(contact_dofs, 0) << "no contact DOFs active - test is not exercising contact";
  const Real physical_its =
      runCumulativeNL("frictionless_physical.i", {"Contact/mortar/c_normal_strategy=physical"});
  EXPECT_LT(physical_its, default_its)
      << "physical c_normal (" << physical_its
      << " cumulative NL iters) should beat default c_normal=1e6 (" << default_its << " iters)";
}

// Verify that c_normal_strategy=physical + c_tangential_strategy=physical reduces
// cumulative nonlinear iterations compared to default c_normal=1e6/c_tangential=1
// for frictional (Coulomb) mortar contact with sliding.
TEST(PhysicalMortarConstants, FrictionalBeatsDefault)
{
  Real contact_dofs = 0;
  const Real default_its = runCumulativeNL("frictional_physical.i", {}, &contact_dofs);
  EXPECT_GT(contact_dofs, 0) << "no contact DOFs active - test is not exercising contact";
  const Real physical_its = runCumulativeNL(
      "frictional_physical.i",
      {"Contact/mortar/c_normal_strategy=physical", "Contact/mortar/c_tangential_strategy=physical"});
  EXPECT_LT(physical_its, default_its)
      << "physical c_normal+c_tangential (" << physical_its
      << " cumulative NL iters) should beat defaults c_normal=1e6/c_tangential=1 (" << default_its
      << " iters)";
}

// Verify that setting c_normal together with c_normal_strategy=physical is an error.
TEST(PhysicalMortarConstants, CNormalSetWithPhysicalThrows)
{
  EXPECT_THROW(
      runCumulativeNL("frictionless_physical.i",
                      {"Contact/mortar/c_normal_strategy=physical", "Contact/mortar/c_normal=1e6"}),
      std::exception);
}

// Verify that c_tangential_strategy=physical is rejected on a frictionless contact pair.
TEST(PhysicalMortarConstants, CTangentialPhysicalOnFrictionlessThrows)
{
  EXPECT_THROW(runCumulativeNL("frictionless_physical.i",
                               {"Contact/mortar/c_tangential_strategy=physical"}),
               std::exception);
}

#endif // MOOSE_AD_MAX_DOFS_PER_ELEM >= 250

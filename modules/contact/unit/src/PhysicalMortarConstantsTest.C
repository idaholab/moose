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
#include "LMWeightedGapUserObject.h"
#include "MooseConfig.h"
#include "MooseMain.h"
#include "NonlinearSystemBase.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

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

struct RunResult
{
  Real contact_dofs;
  Real normal_pressure;
  Real tangential_pressure;
  Real tangential_pressure_two;
  Real top_displacement;
  Real physical_scale;
  Real nonlinear_iterations;
  unsigned int residual_evaluations;
  int time_steps;
  Real kkt_error;
  Real disp_x_residual;
  Real disp_y_residual;
  Real disp_z_residual;
};

static void
recordRun(const std::string & prefix, const RunResult & result)
{
  testing::Test::RecordProperty(prefix + "_solved", result.time_steps > 0);
  testing::Test::RecordProperty(prefix + "_time_steps", result.time_steps);
  testing::Test::RecordProperty(prefix + "_nonlinear_iterations", result.nonlinear_iterations);
  testing::Test::RecordProperty(prefix + "_residual_evaluations", result.residual_evaluations);
  testing::Test::RecordProperty(prefix + "_normal_pressure", result.normal_pressure);
  testing::Test::RecordProperty(prefix + "_tangential_pressure", result.tangential_pressure);
  testing::Test::RecordProperty(prefix + "_tangential_pressure_two",
                                result.tangential_pressure_two);
  testing::Test::RecordProperty(prefix + "_top_displacement", result.top_displacement);
  testing::Test::RecordProperty(prefix + "_kkt_error", result.kkt_error);
  testing::Test::RecordProperty(prefix + "_disp_x_residual", result.disp_x_residual);
  testing::Test::RecordProperty(prefix + "_disp_y_residual", result.disp_y_residual);
  testing::Test::RecordProperty(prefix + "_disp_z_residual", result.disp_z_residual);
}

// Run the given input file with additional CLI args and return physical contact results.
static RunResult
runContact(const std::string & input, const std::vector<std::string> & extra_args)
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
  const bool frictional = input.find("frictional") != std::string::npos;
  const bool unit_2d = input == "frictional_unit_2d.i";
  const bool unit_3d = input == "frictional_unit_3d.i";
  const bool unit_scale = unit_2d || unit_3d;
  Real physical_scale = 0.0;
  if (unit_scale || std::find(extra_args.begin(),
                              extra_args.end(),
                              "Contact/mortar/c_normal_strategy=physical") != extra_args.end())
  {
    const auto & uo = dynamic_cast<const LMWeightedGapUserObject &>(problem.getUserObjectBase(
        frictional ? "lm_weightedvelocities_object_mortar" : "lm_weightedgap_object_mortar"));
    if (uo.dofToDerivedC().empty())
      throw std::runtime_error("physical contact scale map is empty");
    for (const auto & entry : uo.dofToDerivedC())
      physical_scale += entry.second[0];
    physical_scale /= uo.dofToDerivedC().size();
  }
  const auto residual_evaluations = problem.getNonlinearSystemBase(0).nResidualEvaluations();
  return {problem.getPostprocessorValueByName("contact"),
          problem.getPostprocessorValueByName("normal_pressure"),
          frictional ? problem.getPostprocessorValueByName("tangential_pressure") : 0.0,
          unit_3d ? problem.getPostprocessorValueByName("tangential_pressure_two") : 0.0,
          problem.getPostprocessorValueByName("top_displacement"),
          physical_scale,
          problem.getPostprocessorValueByName("cumulative"),
          residual_evaluations,
          problem.timeStep(),
          unit_scale ? problem.getPostprocessorValueByName("kkt_error") : 0.0,
          unit_scale ? problem.getPostprocessorValueByName("disp_x_residual") : 0.0,
          unit_scale ? problem.getPostprocessorValueByName("disp_y_residual") : 0.0,
          unit_3d ? problem.getPostprocessorValueByName("disp_z_residual") : 0.0};
}

// Physical pressure must be independent of the normal multiplier equation scaling.
TEST(PhysicalMortarConstants, FrictionlessRowScalingInvariant)
{
  const std::vector<std::string> physical = {"Contact/mortar/c_normal_strategy=physical"};
  const auto reference = runContact("frictionless_physical.i", physical);
  const auto row_scaled = runContact(
      "frictionless_physical.i",
      {"Contact/mortar/c_normal_strategy=physical", "Contact/mortar/normal_lm_scaling=1e-6"});
  EXPECT_GT(reference.contact_dofs, 0) << "no contact DOFs active - test is not exercising contact";
  EXPECT_NEAR(reference.normal_pressure,
              row_scaled.normal_pressure,
              1e-8 * std::max(1.0, std::abs(reference.normal_pressure)));
  EXPECT_NEAR(reference.top_displacement, row_scaled.top_displacement, 1e-8);
}

// Physical normal and tangential pressures must be independent of their equation scaling.
TEST(PhysicalMortarConstants, FrictionalRowScalingInvariant)
{
  for (const auto & formulation : {"alart_curnier", "hueber_stadler_wohlmuth"})
  {
    const std::string formulation_arg =
        std::string("Contact/mortar/friction_ncp_formulation=") + formulation;
    const std::vector<std::string> common = {"Contact/mortar/c_normal_strategy=physical",
                                             "Contact/mortar/c_tangential_strategy=physical",
                                             "Executioner/abort_on_solve_fail=true",
                                             "Executioner/nl_abs_tol=1e-12",
                                             "Executioner/nl_rel_tol=1e-12",
                                             formulation_arg};
    const auto reference = runContact("frictional_physical.i", common);
    auto row_scaled_args = common;
    row_scaled_args.push_back("Contact/mortar/normal_lm_scaling=1e-6");
    row_scaled_args.push_back("Contact/mortar/tangential_lm_scaling=1e-5");
    const auto row_scaled = runContact("frictional_physical.i", row_scaled_args);
    EXPECT_GT(reference.contact_dofs, 0)
        << "no contact DOFs active - test is not exercising contact";
    EXPECT_NEAR(reference.normal_pressure,
                row_scaled.normal_pressure,
                1e-8 * std::max(1.0, std::abs(reference.normal_pressure)));
    EXPECT_NEAR(reference.tangential_pressure,
                row_scaled.tangential_pressure,
                1e-8 * std::max(1.0, std::abs(reference.tangential_pressure)));
    EXPECT_NEAR(reference.top_displacement, row_scaled.top_displacement, 1e-8);
  }
}

// Both literature formulations must converge without cutbacks to the same physical solution.
TEST(PhysicalMortarConstants, FrictionalFormulationsAgree)
{
  const std::vector<std::string> common = {"Contact/mortar/c_normal_strategy=physical",
                                           "Contact/mortar/c_tangential_strategy=physical",
                                           "Executioner/abort_on_solve_fail=true"};
  auto ac_args = common;
  ac_args.push_back("Contact/mortar/friction_ncp_formulation=alart_curnier");
  auto hsw_args = common;
  hsw_args.push_back("Contact/mortar/friction_ncp_formulation=hueber_stadler_wohlmuth");

  const auto ac = runContact("frictional_physical.i", ac_args);
  const auto hsw = runContact("frictional_physical.i", hsw_args);
  recordRun("alart_curnier", ac);
  recordRun("hsw", hsw);
  EXPECT_EQ(ac.time_steps, 2);
  EXPECT_EQ(hsw.time_steps, 2);
  EXPECT_NEAR(
      ac.normal_pressure, hsw.normal_pressure, 1e-8 * std::max(1.0, std::abs(ac.normal_pressure)));
  EXPECT_NEAR(ac.tangential_pressure,
              hsw.tangential_pressure,
              1e-8 * std::max(1.0, std::abs(ac.tangential_pressure)));
  EXPECT_NEAR(ac.top_displacement, hsw.top_displacement, 1e-8);
}

// At C_n = C_t = 1, user and physical strategies differ only in the source of that scale.
TEST(PhysicalMortarConstants, UserPhysicalScaleSourceEquivalence)
{
  const std::vector<std::string> unit_material = {
      "Materials/tensor/youngs_modulus=0.19166666666666666667",
      "Materials/tensor_1000/youngs_modulus=0.19166666666666666667",
      "Contact/mortar/friction_ncp_formulation=alart_curnier",
      "Executioner/abort_on_solve_fail=true"};
  auto physical_args = unit_material;
  physical_args.push_back("Contact/mortar/c_normal_strategy=physical");
  physical_args.push_back("Contact/mortar/c_tangential_strategy=physical");
  auto user_args = unit_material;
  user_args.push_back("Contact/mortar/normalize_c=true");
  user_args.push_back("Contact/mortar/c_normal=1");
  user_args.push_back("Contact/mortar/c_tangential=1");

  const auto physical = runContact("frictional_physical.i", physical_args);
  const auto user = runContact("frictional_physical.i", user_args);
  EXPECT_NEAR(physical.physical_scale, 1.0, 1e-12);
  EXPECT_EQ(physical.time_steps, 2);
  EXPECT_EQ(user.time_steps, 2);
  EXPECT_NEAR(physical.normal_pressure,
              user.normal_pressure,
              1e-8 * std::max(1.0, std::abs(physical.normal_pressure)));
  EXPECT_NEAR(physical.tangential_pressure,
              user.tangential_pressure,
              1e-8 * std::max(1.0, std::abs(physical.tangential_pressure)));
  EXPECT_NEAR(physical.top_displacement, user.top_displacement, 1e-8);
}

// Unnormalized user constants contain the current mortar weight and may update between residuals.
TEST(PhysicalMortarConstants, UnnormalizedUserScaleUpdates)
{
  const auto result = runContact("frictional_physical.i",
                                 {"Contact/mortar/c_normal=2e7",
                                  "Contact/mortar/c_tangential=2e7",
                                  "Executioner/abort_on_solve_fail=true"});
  EXPECT_EQ(result.time_steps, 2);
  EXPECT_GT(result.contact_dofs, 0) << "no contact DOFs active - test is not exercising contact";
  EXPECT_TRUE(std::isfinite(result.normal_pressure));
  EXPECT_TRUE(std::isfinite(result.tangential_pressure));
}

// Both formulations must satisfy the same physical KKT and equilibrium criteria at unit scale.
TEST(PhysicalMortarConstants, FrictionalUnitScaleSliding)
{
  const std::vector<std::string> common = {"Executioner/abort_on_solve_fail=true"};
  auto ac_args = common;
  ac_args.push_back("Contact/mortar/friction_ncp_formulation=alart_curnier");
  auto hsw_args = common;
  hsw_args.push_back("Contact/mortar/friction_ncp_formulation=hueber_stadler_wohlmuth");

  const auto ac = runContact("frictional_unit_2d.i", ac_args);
  const auto hsw = runContact("frictional_unit_2d.i", hsw_args);
  recordRun("alart_curnier", ac);
  recordRun("hsw", hsw);
  EXPECT_EQ(ac.time_steps, 2);
  EXPECT_EQ(hsw.time_steps, 2);
  EXPECT_NEAR(ac.physical_scale, 1.0, 1e-12);
  EXPECT_NEAR(hsw.physical_scale, 1.0, 1e-12);
  EXPECT_LT(ac.kkt_error, 1e-10);
  EXPECT_LT(hsw.kkt_error, 1e-10);
  EXPECT_LT(ac.disp_x_residual, 1e-10);
  EXPECT_LT(ac.disp_y_residual, 1e-10);
  EXPECT_LT(hsw.disp_x_residual, 1e-10);
  EXPECT_LT(hsw.disp_y_residual, 1e-10);
  EXPECT_NEAR(ac.normal_pressure, hsw.normal_pressure, 1e-8);
  EXPECT_NEAR(ac.tangential_pressure, hsw.tangential_pressure, 1e-8);
  EXPECT_NEAR(ac.top_displacement, hsw.top_displacement, 1e-8);
}

// Cold activation, sticking, and separation must use the same roots in both formulations.
TEST(PhysicalMortarConstants, FrictionalUnitScaleStickAndSeparate)
{
  std::vector<RunResult> stick;
  std::vector<RunResult> open;
  for (const auto & formulation : {"alart_curnier", "hueber_stadler_wohlmuth"})
  {
    const std::string formulation_arg =
        std::string("Contact/mortar/friction_ncp_formulation=") + formulation;
    stick.push_back(
        runContact("frictional_unit_2d.i", {formulation_arg, "tangential_load=0.01*t"}));
    open.push_back(runContact("frictional_unit_2d.i",
                              {formulation_arg, "tangential_load=0", "normal_load=0.1*t"}));
  }

  for (const auto & result : stick)
  {
    EXPECT_GT(result.contact_dofs, 0);
    EXPECT_LT(std::abs(result.tangential_pressure), 0.5 * result.normal_pressure);
    EXPECT_LT(result.kkt_error, 1e-10);
    EXPECT_LT(result.disp_x_residual, 1e-10);
    EXPECT_LT(result.disp_y_residual, 1e-10);
  }
  for (const auto & result : open)
  {
    EXPECT_EQ(result.contact_dofs, 0);
    EXPECT_NEAR(result.normal_pressure, 0.0, 1e-12);
    EXPECT_NEAR(result.tangential_pressure, 0.0, 1e-12);
    EXPECT_LT(result.kkt_error, 1e-10);
  }
  EXPECT_NEAR(stick[0].normal_pressure, stick[1].normal_pressure, 1e-8);
  EXPECT_NEAR(stick[0].tangential_pressure, stick[1].tangential_pressure, 1e-8);
  EXPECT_NEAR(open[0].top_displacement, open[1].top_displacement, 1e-8);
  recordRun("alart_curnier_stick", stick[0]);
  recordRun("hsw_stick", stick[1]);
  recordRun("alart_curnier_open", open[0]);
  recordRun("hsw_open", open[1]);
}

// Fixed load increments and multiplier initial guesses must not change the physical endpoint.
TEST(PhysicalMortarConstants, FrictionalUnitScaleRobustnessSweep)
{
  for (const auto & formulation : {"alart_curnier", "hueber_stadler_wohlmuth"})
  {
    const std::string formulation_arg =
        std::string("Contact/mortar/friction_ncp_formulation=") + formulation;
    const auto reference = runContact("frictional_unit_2d.i", {formulation_arg});
    recordRun(std::string(formulation) + "_one_increment", reference);
    unsigned int residual_evaluations = reference.residual_evaluations;

    for (const auto & increment : std::vector<std::pair<std::string, int>>{{"0.5", 3}, {"0.25", 5}})
    {
      const auto result = runContact("frictional_unit_2d.i",
                                     {formulation_arg,
                                      "Executioner/dt=" + increment.first,
                                      "Executioner/dtmin=" + increment.first});
      EXPECT_EQ(result.time_steps, increment.second);
      EXPECT_LT(result.kkt_error, 1e-10);
      EXPECT_NEAR(result.normal_pressure, reference.normal_pressure, 1e-8);
      EXPECT_NEAR(result.tangential_pressure, reference.tangential_pressure, 1e-8);
      EXPECT_NEAR(result.top_displacement, reference.top_displacement, 1e-8);
      recordRun(std::string(formulation) + "_" + std::to_string(increment.second - 1) +
                    "_increments",
                result);
      residual_evaluations += result.residual_evaluations;
    }

    const auto plausible =
        runContact("frictional_unit_2d.i",
                   {formulation_arg, "normal_lm_guess=0.05", "tangential_lm_guess=0.02"});
    EXPECT_EQ(plausible.time_steps, 2);
    EXPECT_LT(plausible.kkt_error, 1e-10);
    EXPECT_NEAR(plausible.normal_pressure, reference.normal_pressure, 1e-8);
    EXPECT_NEAR(plausible.tangential_pressure, reference.tangential_pressure, 1e-8);
    EXPECT_NEAR(plausible.top_displacement, reference.top_displacement, 1e-8);
    recordRun(std::string(formulation) + "_plausible_guess", plausible);
    residual_evaluations += plausible.residual_evaluations;

    const auto poor = runContact("frictional_unit_2d.i",
                                 {formulation_arg, "normal_lm_guess=5", "tangential_lm_guess=-5"});
    recordRun(std::string(formulation) + "_poor_guess", poor);
    testing::Test::RecordProperty(std::string(formulation) + "_poor_guess_solved",
                                  poor.time_steps == 2);
    residual_evaluations += poor.residual_evaluations;
    testing::Test::RecordProperty(std::string(formulation) + "_residual_evaluations",
                                  residual_evaluations);
  }
}

// Exercise close, stick, slide, reverse, open, and reclose without time-step cutbacks.
TEST(PhysicalMortarConstants, FrictionalUnitScaleLoadHistory)
{
  std::vector<RunResult> results;
  for (const auto & formulation : {"alart_curnier", "hueber_stadler_wohlmuth"})
    results.push_back(
        runContact("frictional_unit_2d.i",
                   {std::string("Contact/mortar/friction_ncp_formulation=") + formulation,
                    "tangential_load=tangential_history",
                    "normal_load=normal_history",
                    "Executioner/end_time=16"}));

  for (const auto & result : results)
  {
    EXPECT_EQ(result.time_steps, 17);
    EXPECT_GT(result.contact_dofs, 0);
    EXPECT_LT(result.kkt_error, 1e-10);
    EXPECT_LT(result.disp_x_residual, 1e-10);
    EXPECT_LT(result.disp_y_residual, 1e-10);
  }
  EXPECT_NEAR(results[0].normal_pressure, results[1].normal_pressure, 1e-8);
  EXPECT_NEAR(results[0].tangential_pressure, results[1].tangential_pressure, 1e-8);
  EXPECT_NEAR(results[0].top_displacement, results[1].top_displacement, 1e-8);
  recordRun("alart_curnier", results[0]);
  recordRun("hsw", results[1]);
}

// Confirm two-component projection and a change in slip direction on a 3-D unit-scale interface.
TEST(PhysicalMortarConstants, FrictionalUnitScaleThreeDimensional)
{
  std::vector<RunResult> results;
  for (const auto & formulation : {"alart_curnier", "hueber_stadler_wohlmuth"})
    results.push_back(
        runContact("frictional_unit_3d.i",
                   {std::string("Contact/mortar/friction_ncp_formulation=") + formulation,
                    "tangential_x_load=tangential_x_history",
                    "tangential_y_load=tangential_y_history",
                    "normal_load=normal_history",
                    "Executioner/end_time=3"}));

  for (const auto & result : results)
  {
    EXPECT_EQ(result.time_steps, 4);
    EXPECT_GT(result.contact_dofs, 0);
    EXPECT_NEAR(result.physical_scale, 1.0, 1e-12);
    EXPECT_LT(result.kkt_error, 1e-10);
    EXPECT_LT(result.disp_x_residual, 1e-10);
    EXPECT_LT(result.disp_y_residual, 1e-10);
    EXPECT_LT(result.disp_z_residual, 1e-10);
  }
  EXPECT_NEAR(results[0].normal_pressure, results[1].normal_pressure, 1e-8);
  EXPECT_NEAR(results[0].tangential_pressure, results[1].tangential_pressure, 1e-8);
  EXPECT_NEAR(results[0].tangential_pressure_two, results[1].tangential_pressure_two, 1e-8);
  EXPECT_NEAR(results[0].top_displacement, results[1].top_displacement, 1e-8);
  recordRun("alart_curnier", results[0]);
  recordRun("hsw", results[1]);
}

// Backtracking must globalize active-set changes on a normal-refined frictional mesh.
TEST(PhysicalMortarConstants, FrictionalNormalRefinementConverges)
{
  const auto result = runContact("frictional_physical.i",
                                 {"Contact/mortar/c_normal_strategy=physical",
                                  "Contact/mortar/c_tangential_strategy=physical",
                                  "Mesh/top_block/nz=6"});
  EXPECT_GT(result.contact_dofs, 0) << "no contact DOFs active - test is not exercising contact";
  EXPECT_TRUE(std::isfinite(result.normal_pressure));
  EXPECT_TRUE(std::isfinite(result.tangential_pressure));
}

// Field-split preconditioning must see the right-scaled assembled preconditioning matrix.
TEST(PhysicalMortarConstants, FrictionalFieldSplitConverges)
{
  const auto result = runContact("frictional_physical.i",
                                 {"Contact/mortar/c_normal_strategy=physical",
                                  "Contact/mortar/c_tangential_strategy=physical",
                                  "Preconditioning/active=fsp"});
  EXPECT_GT(result.contact_dofs, 0) << "no contact DOFs active - test is not exercising contact";
  EXPECT_TRUE(std::isfinite(result.normal_pressure));
  EXPECT_TRUE(std::isfinite(result.tangential_pressure));
}

// Q is an inverse series compliance based on normal element depths, not tangential resolution.
TEST(PhysicalMortarConstants, SeriesComplianceScale)
{
  const std::vector<std::string> physical = {"Contact/mortar/c_normal_strategy=physical",
                                             "BCs/topz/function=0*${starting_point}+${offset}",
                                             "Executioner/end_time=.025"};
  const auto reference = runContact("frictionless_physical.i", physical);
  auto normal_refinement_args = physical;
  normal_refinement_args.push_back("Mesh/top_block/nz=6");
  const auto normal_refinement = runContact("frictionless_physical.i", normal_refinement_args);
  auto tangential_refinement_args = physical;
  tangential_refinement_args.push_back("Mesh/top_block/nx=6");
  const auto tangential_refinement =
      runContact("frictionless_physical.i", tangential_refinement_args);

  const Real primary_depth = 0.05 / 2.0;
  const auto expected = [primary_depth](const Real secondary_depth)
  { return 1.0 / (secondary_depth / 1.0e4 + primary_depth / 1.0e5); };
  EXPECT_NEAR(reference.physical_scale, expected(0.5 / 3.0), 1e-10 * expected(0.5 / 3.0));
  EXPECT_NEAR(normal_refinement.physical_scale, expected(0.5 / 6.0), 1e-10 * expected(0.5 / 6.0));
  EXPECT_NEAR(tangential_refinement.physical_scale,
              reference.physical_scale,
              1e-10 * reference.physical_scale);
}

// Verify that setting c_normal together with c_normal_strategy=physical is an error.
TEST(PhysicalMortarConstants, CNormalSetWithPhysicalThrows)
{
  EXPECT_THROW(
      runContact("frictionless_physical.i",
                 {"Contact/mortar/c_normal_strategy=physical", "Contact/mortar/c_normal=1e6"}),
      std::exception);
}

// Verify that c_tangential_strategy=physical is rejected on a frictionless contact pair.
TEST(PhysicalMortarConstants, CTangentialPhysicalOnFrictionlessThrows)
{
  EXPECT_THROW(
      runContact("frictionless_physical.i", {"Contact/mortar/c_tangential_strategy=physical"}),
      std::exception);
}

// Physical tangential scaling needs the normal physical scale used in its projection.
TEST(PhysicalMortarConstants, CTangentialPhysicalWithoutNormalThrows)
{
  EXPECT_THROW(
      runContact("frictional_physical.i", {"Contact/mortar/c_tangential_strategy=physical"}),
      std::exception);
}

// Dynamic mortar uses different equations and is not supported by this implementation.
TEST(PhysicalMortarConstants, PhysicalMortarDynamicsThrows)
{
  EXPECT_THROW(runContact("frictionless_physical.i",
                          {"Contact/mortar/c_normal_strategy=physical",
                           "Contact/mortar/mortar_dynamics=true"}),
               std::exception);
}

// Series compliance requires positive directional material stiffness on both bodies.
TEST(PhysicalMortarConstants, InvalidPhysicalStiffnessThrows)
{
  EXPECT_THROW(runContact("frictionless_physical.i",
                          {"Contact/mortar/c_normal_strategy=physical",
                           "Materials/tensor/youngs_modulus=-1"}),
               std::exception);
}

// User-supplied nodal pressure scales must be positive and finite.
TEST(PhysicalMortarConstants, InvalidUserScaleThrows)
{
  for (const auto & value : {"0", "nan"})
  {
    EXPECT_THROW(runContact("frictionless_physical.i",
                            {"Contact/mortar/c_normal_strategy=user",
                             "Executioner/abort_on_solve_fail=true",
                             std::string("Contact/mortar/c_normal=") + value}),
                 std::exception);
    EXPECT_THROW(runContact("frictional_physical.i",
                            {"Contact/mortar/c_normal_strategy=user",
                             "Contact/mortar/c_tangential_strategy=user",
                             "Contact/mortar/c_normal=1",
                             "Executioner/abort_on_solve_fail=true",
                             std::string("Contact/mortar/c_tangential=") + value}),
                 std::exception);
  }
}

#endif // MOOSE_AD_MAX_DOFS_PER_ELEM >= 250

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
#include "FEProblemBase.h"
#include "ExecutablePath.h"
#include "MooseApp.h"
#include "MooseTypes.h"
#include "NewtonSNESExecutor.h"

#include <petscsnes.h>

#include <filesystem>
#include <string>
#include <system_error>
#include <vector>

namespace
{
struct SolveCase
{
  std::string name;
  std::string input;
  std::string outer_executor;
  unsigned int max_outer_iterations;
  Real expected_u;
  Real expected_v;
  Real postprocessor_tolerance;
  std::vector<std::string> cli_args = {};
};

class NLPPreconditionerSolveTest : public testing::TestWithParam<SolveCase>
{
};

class ScopedCurrentPath
{
public:
  ScopedCurrentPath() : _original_path(std::filesystem::current_path()) {}
  ~ScopedCurrentPath()
  {
    std::error_code ec;
    std::filesystem::current_path(_original_path, ec);
  }

private:
  const std::filesystem::path _original_path;
};

std::string
unitInputPath(const std::string & input)
{
  return (std::filesystem::path(Moose::getExecutablePath()) / input).string();
}

std::string
testName(const testing::TestParamInfo<SolveCase> & info)
{
  return info.param.name;
}
}

TEST_P(NLPPreconditionerSolveTest, SolvesWithExpectedPhysicsAndOuterIterations)
{
  const auto & test_case = GetParam();
  const ScopedCurrentPath scoped_current_path;

  std::vector<std::string> args = {"--executor", "-i", unitInputPath(test_case.input)};
  args.insert(args.end(), test_case.cli_args.begin(), test_case.cli_args.end());

  auto app = AppFactory::create("MooseUnitApp", args);
  app->run();

  auto & fe_problem = app->feProblem();

  EXPECT_NEAR(fe_problem.getPostprocessorValueByName("u"),
              test_case.expected_u,
              test_case.postprocessor_tolerance);
  EXPECT_NEAR(fe_problem.getPostprocessorValueByName("v"),
              test_case.expected_v,
              test_case.postprocessor_tolerance);

  auto & outer = app->getExecutor<NewtonSNESExecutor>(test_case.outer_executor);
  PetscInt outer_iterations = -1;
  PetscCallAbort(PETSC_COMM_WORLD, SNESGetIterationNumber(outer.getSNES(), &outer_iterations));
  ASSERT_GE(outer_iterations, 0);
  EXPECT_LE(static_cast<unsigned int>(outer_iterations), test_case.max_outer_iterations);
}

INSTANTIATE_TEST_SUITE_P(
    Cases,
    NLPPreconditionerSolveTest,
    testing::Values(SolveCase{"nlp_basic",
                              "files/NLPPreconditionerSolveTest/nlp_basic.i",
                              "outer",
                              1,
                              0.5,
                              0.5,
                              1e-10},
                    SolveCase{"off_diag_coupling_nlp",
                              "files/NLPPreconditionerSolveTest/off_diag_coupling_nlp.i",
                              "outer",
                              1,
                              0.544316318,
                              0.544316318,
                              1e-10},
                    SolveCase{"one_dof_block_nlp_multiplicative",
                              "files/NLPPreconditionerSolveTest/one_dof_block_nlp.i",
                              "outer",
                              1,
                              1.0,
                              1.0,
                              1e-10},
                    SolveCase{"one_dof_block_nlp_symmetric_multiplicative",
                              "files/NLPPreconditionerSolveTest/one_dof_block_nlp.i",
                              "outer",
                              1,
                              1.0,
                              1.0,
                              1e-10,
                              {"Executors/shell_pc/sweep_type=symmetric_multiplicative"}},
                    SolveCase{"keyes_mspin_ex2",
                              "files/NLPPreconditionerSolveTest/keyes-mspin-ex2/mspin.i",
                              "newton",
                              4,
                              0.010544263113441546,
                              0.10825554366536486,
                              1e-10}),
    testName);

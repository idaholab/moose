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
#include "MooseUnitUtils.h"

std::unique_ptr<MooseApp>
createApp(const std::vector<std::string> & cli_args)
{
  return AppFactory::create("MooseUnitApp", cli_args);
};

TEST(MooseAppParamsTest, checkExclusiveParams)
{
  MOOSE_ASSERT_THROWS_CONTAINS(
      MooseRuntimeError,
      createApp({"--check-input", "--recover"}),
      "--check-input: Cannot be used with parameter '--recover'; recover files might not exist");
  MOOSE_ASSERT_THROWS_CONTAINS(
      MooseRuntimeError,
      createApp({"--test-restep", "--test-checkpoint-half-transient"}),
      "--test-restep: Cannot be used with parameter '--test-checkpoint-half-transient'");
  MOOSE_ASSERT_THROWS_CONTAINS(MooseRuntimeError,
                               createApp({"--trap-fpe", "--no-trap-fpe"}),
                               "--trap-fpe: Cannot be used with parameter '--no-trap-fpe'");
}

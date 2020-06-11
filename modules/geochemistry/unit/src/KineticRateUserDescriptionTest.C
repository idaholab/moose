//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "KineticRateUserDescription.h"

/// Test exceptions
TEST(KineticRateUserDescriptionTest, exceptions)
{

  try
  {
    KineticRateUserDescription rate(
        "CH4(aq)", 1.0, 2.0, true, {"H2O", "H+"}, {3.0}, 4.0, 5.0, 6.0, 7.0);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find("The promoting_species and promoting_indices vectors must be the same size") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    KineticRateUserDescription rate(
        "CH4(aq)", 1.0, 2.0, true, {"H2O", "OH-", "H2O"}, {3.0, 1.0, -1.0}, 4.0, 5.0, 6.0, 7.0);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Promoting species H2O has already been provided with an exponent") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

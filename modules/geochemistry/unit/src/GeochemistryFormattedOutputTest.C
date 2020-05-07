//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "GeochemistryFormattedOutput.h"

TEST(GeochemistryFormattedOutput, excepts)
{
  DenseMatrix<Real> stoi(1, 3);

  try
  {
    std::string s = GeochemistryFormattedOutput::reaction(stoi, 1, {"name"}, 1.0, 1);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("GeochemistryFormattedOutput::reaction called with stoichiometric matrix "
                         "having 1 rows, but row = 1") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    std::string s = GeochemistryFormattedOutput::reaction(stoi, 0, {"name", "name2"}, 1.0, 1);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("GeochemistryFormattedOutput::reaction called with stoichiometric matrix "
                         "having 3 columns, but names has size 2") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

TEST(GeochemistryFormattedOutput, reactionFormat)
{
  DenseMatrix<Real> stoi(2, 3);
  stoi(0, 0) = -1.0;
  stoi(0, 1) = 1E-3;
  stoi(0, 2) = -1.0 / 3.0;
  stoi(1, 0) = -1E-3;
  stoi(1, 1) = 2.0;
  stoi(1, 2) = 456789.0;
  EXPECT_EQ(GeochemistryFormattedOutput::reaction(stoi, 0, {"n0", "n1", "n2"}, 1.0E-2, 3),
            "-1*n0 - 0.333*n2");
  EXPECT_EQ(GeochemistryFormattedOutput::reaction(stoi, 1, {"n0", "n1", "n2"}, 1.0E-2, 3),
            "2*n1 + 4.57e+05*n2");
}

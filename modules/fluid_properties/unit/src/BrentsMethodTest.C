//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"
#include "BrentsMethod.h"

/**
 * Test function for Brents method.
 * f(x) = log(1 + x) * tanh(x / 3) + x / 4 - 3 which has a root at 5.170302597.
 *
 * Note: the implementation of Brents bracketing method restricts the bracketing
 * interval to positive values
 */
Real
f(Real x)
{
  return std::log(1.0 + x) * std::tanh(x / 3.0) + x / 4.0 - 3;
}

TEST(BrentsMethod, bracket)
{
  // Initial guess for bracketing interval does not bracket root
  Real x1 = 0.5;
  Real x2 = 1.0;

  auto func = [](Real x) { return f(x); };

  // Call bracket to determine the bracketing interval
  BrentsMethod::bracket(func, x1, x2);

  // The product of the function f(x) at the bracketing interval (x1, x2) must
  // be negative to bracket a root
  EXPECT_LT(f(x1) * f(x2), 0.0);

  // Test that a warning is thrown if the initial guesses are equal
  try
  {
    // Trigger identical initial guess error
    BrentsMethod::bracket(func, x1, x1);
    FAIL() << "missing expected exception";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Bad initial range (0) used in BrentsMethod::bracket") !=
                std::string::npos)
        << "failed with unexpected error: " << msg;
  }

  // Test that a warning is thrown if no bracketing interval is found after 50 iterations.
  try
  {
    // Trigger no bracketing interval warning by adding 4 to f(x), whereby no real root exists
    auto func2 = [](Real x) { return f(x) + 4.0; };

    BrentsMethod::bracket(func2, x1, x2);
    FAIL() << "missing expected exception";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find("No bracketing interval found by BrentsMethod::bracket after 50 iterations") !=
        std::string::npos)
        << "failed with unexpected error: " << msg;
  }
}

TEST(BrentsMethod, root)
{
  // Bracketing interval that does bracket root
  Real x1 = 0.5;
  Real x2 = 10.0;

  auto func = [](Real x) { return f(x); };

  // Check that the root is 5.170302597
  EXPECT_NEAR(5.170302597, BrentsMethod::root(func, x1, x2), 1e-8);

  // Test that a warning is thrown if the supplied interval does not bracket the root
  try
  {
    // Trigger no bracketing interval error
    x2 = 1.0;
    BrentsMethod::root(func, x1, x2);
    FAIL() << "missing expected exception";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Root must be bracketed in BrentsMethod::root") != std::string::npos)
        << "failed with unexpected error: " << msg;
  }
}

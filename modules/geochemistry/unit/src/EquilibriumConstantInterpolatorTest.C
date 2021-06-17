//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "EquilibriumConstantInterpolator.h"

const double tol = 1.0e-6;

/// exception tests for constructor
TEST(EquilibriumConstantInterpolatorTest, constructor_except)
{
  // Fourth-order polynomial
  std::vector<double> T = {0.0, 25.0};
  std::vector<double> k_bad = {-6.5570};
  std::vector<double> k_good = {-6.5570, 1.0};

  try
  {
    EquilibriumConstantInterpolator logk(T, k_bad, "fourth-order");
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("The temperature and logk data sets must be equal in length in "
                         "EquilibriumConstantInterpolator") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    EquilibriumConstantInterpolator logk(T, k_good, "bad-type");
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Type bad-type is not supported in EquilibriumConstantInterpolator") !=
                std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }

  try
  {
    EquilibriumConstantInterpolator logk(T, k_good, "maier-kelly");
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find("A Maier-Kelly fit cannot be used when the temperature points include 0. Use a "
                 "fourth-order fit instead") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

TEST(EquilibriumConstantInterpolatorTest, constructor)
{
  // Fourth-order polynomial
  std::vector<double> T = {0.0, 25.0, 60.0, 100.0, 150.0, 200.0, 250.0, 300.0};
  std::vector<double> k = {-6.5570, -6.3660, -6.3325, -6.4330, -6.7420, -7.1880, -7.7630, -8.4650};

  EquilibriumConstantInterpolator logk(T, k, "fourth-order");
  EXPECT_EQ(logk.getSampleSize(), T.size());

  // Maier-Kelly fit
  T = {0.01, 25.0, 60.0, 100.0, 150.0, 200.0, 250.0, 300.0};
  k = {-6.5570, -6.3660, -6.3325, -6.4330, -6.7420, -7.1880, -7.7630, -8.4650};

  EquilibriumConstantInterpolator logk2(T, k, "maier-kelly");
  EXPECT_EQ(logk2.getSampleSize(), T.size());

  // Piecewise-linear fit
  EquilibriumConstantInterpolator logk3(T, k, "piecewise-linear");
  EXPECT_EQ(logk3.getSampleSize(), T.size());
}

TEST(EquilibriumConstantInterpolatorTest, linear)
{
  const std::vector<double> T = {0.0, 25.0};
  const std::vector<double> k = {1.2, 2.4};

  // Instantiate with fourth-order fit, but a linear fit will be constructed as there
  // are not enought points to fit a fourth-order polynomial
  EquilibriumConstantInterpolator logk(T, k, "fourth-order");
  EXPECT_EQ(logk.getSampleSize(), T.size());

  logk.generate();

  EXPECT_NEAR(logk.sample(T[0]), 1.2, tol);
  EXPECT_NEAR(logk.sample(T[1]), 2.4, tol);
  EXPECT_NEAR(logk.sample(50.0), 3.6, tol);

  EXPECT_NEAR(logk.sampleDerivative(50.0), 0.048, tol);

  // Check sampleDerivative() with the AD version
  DualReal Tad = 50.0;
  Moose::derivInsert(Tad.derivatives(), 0, 1.0);
  EXPECT_NEAR(logk.sampleDerivative(50.0), logk.sample(Tad).derivatives()[0], tol);

  // Should get identical results specifying a maier-kelly fit as only a linear
  // fit can be used (note: shift both temperature points to keep linear fit the same
  // as above)
  const std::vector<double> T2 = {0.01, 25.01};
  EquilibriumConstantInterpolator logk2(T2, k, "maier-kelly");
  EXPECT_EQ(logk2.getSampleSize(), T2.size());

  logk2.generate();

  EXPECT_NEAR(logk2.sample(T2[0]), 1.2, tol);
  EXPECT_NEAR(logk2.sample(T2[1]), 2.4, tol);
  EXPECT_NEAR(logk2.sample(50.01), 3.6, tol);

  EXPECT_NEAR(logk2.sampleDerivative(50.0), 0.048, tol);

  // Check sampleDerivative() with the AD version
  DualReal Tad2 = 50.01;
  Moose::derivInsert(Tad2.derivatives(), 0, 1.0);
  EXPECT_NEAR(logk.sampleDerivative(50.01), logk.sample(Tad2).derivatives()[0], tol);
}

TEST(EquilibriumConstantInterpolatorTest, fourthOrder)
{
  const std::vector<double> T = {0.0, 25.0, 60.0, 100.0, 150.0, 200.0, 250.0, 300.0};
  const std::vector<double> k = {
      -6.5570, -6.3660, -6.3325, -6.4330, -6.7420, -7.1880, -7.7630, -8.4650};

  EquilibriumConstantInterpolator logk(T, k, "fourth-order");
  logk.generate();

  /**
   * Compare with values calculated using scipy.optimize.curve_fit
   *
   * from scipy.optimize import curve_fit
   *
   * def logk(T, a0, a1, a2, a3, a4):
   *   return a0 + a1 * T + a2 * T**2 + a3 * T**3 + a4 * T**4
   *
   * T = [0.0, 25.0, 60.0, 100.0, 150.0, 200.0, 250.0, 300.0]
   * k = [-6.5570, -6.3660, -6.3325, -6.4330, -6.7420, -7.1880, -7.7630, -8.4650]
   *
   * popt, pcov = curve_fit(logk, T, k)
   *
   * print(logk(T[1], *popt))
   * print(logk(T[2], *popt))
   * print(logk(T[3], *popt))
   * print(logk(T[4], *popt))
   * print(logk(T[5], *popt))
   * print(logk(125, *popt))
   */
  EXPECT_NEAR(logk.sample(T[1]), -6.38384537894225, tol);
  EXPECT_NEAR(logk.sample(T[2]), -6.321712110081394, tol);
  EXPECT_NEAR(logk.sample(T[3]), -6.429646151221264, tol);
  EXPECT_NEAR(logk.sample(T[4]), -6.744733784802997, tol);
  EXPECT_NEAR(logk.sample(T[5]), -7.193821631774174, tol);
  EXPECT_NEAR(logk.sample(125.0), -6.567092778770316, tol);

  // Check sampleDerivative() with the AD version
  DualReal T2 = 50.0;
  Moose::derivInsert(T2.derivatives(), 0, 1.0);
  EXPECT_NEAR(logk.sampleDerivative(50.0), logk.sample(T2).derivatives()[0], tol);
}

TEST(EquilibriumConstantInterpolatorTest, maierKelly)
{
  const std::vector<double> T = {0.01, 25.0, 60.0, 100.0, 150.0, 200.0, 250.0, 300.0};
  const std::vector<double> k = {
      0.2081, 0.0579, -0.2746, -0.7311, -1.3659, -2.0618, -2.8403, -3.7681};

  EquilibriumConstantInterpolator logk(T, k, "maier-kelly");
  logk.generate();

  /**
   * Compare with values calculated using scipy.optimize.curve_fit
   *
   * from scipy.optimize import curve_fit
   * import numpy as np
   *
   * def logk(T, a0, a1, a2, a3, a4):
   *    return a0 * np.log(T) + a1  + a2 * T + a3 / T + a4 / T**2
   *
   * T = [0.01, 25.0, 60.0, 100.0, 150.0, 200.0, 250.0, 300.0]
   * k = [0.2081, 0.0579, -0.2746, -0.7311, -1.3659, -2.0618, -2.8403, -3.7681]
   *
   * popt, pcov = curve_fit(logk, T, k)
   *
   * print(logk(T[1], *popt))
   * print(logk(T[2], *popt))
   * print(logk(T[3], *popt))
   * print(logk(T[4], *popt))
   * print(logk(T[5], *popt))
   */
  EXPECT_NEAR(logk.sample(T[1]), 0.061497433011205596, tol);
  EXPECT_NEAR(logk.sample(T[2]), -0.30083706818706746, tol);
  EXPECT_NEAR(logk.sample(T[3]), -0.7005921220757013, tol);
  EXPECT_NEAR(logk.sample(T[4]), -1.3418011239865, tol);
  EXPECT_NEAR(logk.sample(T[5]), -2.0827775371250423, tol);
  EXPECT_NEAR(logk.sample(125.0), -1.0053697469019622, tol);

  // Check sampleDerivative() with the AD version
  DualReal T2 = 50.0;
  Moose::derivInsert(T2.derivatives(), 0, 1.0);
  EXPECT_NEAR(logk.sampleDerivative(50.0), logk.sample(T2).derivatives()[0], tol);
}

TEST(EquilibriumConstantInterpolatorTest, linearNullValues)
{
  const std::vector<double> T = {0.0, 25.0, 60.0, 100.0, 150.0, 200.0, 250.0, 300.0};
  const std::vector<double> k = {17.8660, 15.9746, 500.0, 500.0, 500.0, 500.0, 500.0, 500.0};

  EquilibriumConstantInterpolator logk(T, k, "fourth-order");
  logk.generate();

  // Should only be two samples in this case (ignoring values of 500)
  EXPECT_EQ(logk.getSampleSize(), (std::size_t)2);

  // Compare with values calculated using linear fit and extrapolating
  EXPECT_NEAR(logk.sample(T[0]), 17.866, tol);
  EXPECT_NEAR(logk.sample(T[1]), 15.9746, tol);
  EXPECT_NEAR(logk.sample(60.0), 13.32664, tol);
  EXPECT_NEAR(logk.sample(125.0), 8.409, tol);
  EXPECT_NEAR(logk.sample(300.0), -4.8308, tol);
}

TEST(EquilibriumConstantInterpolatorTest, fourthOrderNullValues)
{
  const std::vector<double> T = {0.0, 25.0, 60.0, 100.0, 150.0, 200.0, 250.0, 300.0};
  const std::vector<double> k = {
      -0.0162, -0.0264, -0.0340, -0.0459, -0.0618, -0.0803, 500.0, 500.0};

  EquilibriumConstantInterpolator logk(T, k, "fourth-order");
  logk.generate();

  // Should only be six samples in this case (ignoring values of 500)
  EXPECT_EQ(logk.getSampleSize(), (std::size_t)6);

  // Compare with values calculated using fourth order polynomial (python code above)
  EXPECT_NEAR(logk.sample(T[0]), -0.016418224486741174, tol);
  EXPECT_NEAR(logk.sample(T[1]), -0.025715932916029558, tol);
  EXPECT_NEAR(logk.sample(T[2]), -0.03492782535210691, tol);
  EXPECT_NEAR(logk.sample(T[3]), -0.04524532642418216, tol);
  EXPECT_NEAR(logk.sample(T[4]), -0.0620327728353172, tol);
  EXPECT_NEAR(logk.sample(250.0), -0.08739490878982198, tol);
  EXPECT_NEAR(logk.sample(300.0), -0.060966149023565674, tol);

  // Check sampleDerivative() with the AD version
  DualReal T2 = 50.0;
  Moose::derivInsert(T2.derivatives(), 0, 1.0);
  EXPECT_NEAR(logk.sampleDerivative(50.0), logk.sample(T2).derivatives()[0], tol);
}

TEST(EquilibriumConstantInterpolatorTest, fourthOrderUserDefinedNullValues)
{
  const std::vector<double> T = {0.0, 25.0, 60.0, 100.0, 150.0, 200.0, 250.0, 300.0};
  const std::vector<double> k = {
      -0.0162, -0.0264, -0.0340, -0.0459, -0.0618, -0.0803, 999.999, 999.999};

  EquilibriumConstantInterpolator logk(T, k, "fourth-order", 999.999);
  logk.generate();

  // Should only be six samples in this case (ignoring values of 500)
  EXPECT_EQ(logk.getSampleSize(), (std::size_t)6);

  // Compare with values calculated using fourth order polynomial (python code above)
  EXPECT_NEAR(logk.sample(T[0]), -0.016418224486741174, tol);
  EXPECT_NEAR(logk.sample(T[1]), -0.025715932916029558, tol);
  EXPECT_NEAR(logk.sample(T[2]), -0.03492782535210691, tol);
  EXPECT_NEAR(logk.sample(T[3]), -0.04524532642418216, tol);
  EXPECT_NEAR(logk.sample(T[4]), -0.0620327728353172, tol);
  EXPECT_NEAR(logk.sample(250.0), -0.08739490878982198, tol);
  EXPECT_NEAR(logk.sample(300.0), -0.060966149023565674, tol);

  // Check sampleDerivative() with the AD version
  DualReal T2 = 50.0;
  Moose::derivInsert(T2.derivatives(), 0, 1.0);
  EXPECT_NEAR(logk.sampleDerivative(50.0), logk.sample(T2).derivatives()[0], tol);
}

/// Piecewise-linear
TEST(EquilibriumConstantInterpolatorTest, piecewiselinear)
{
  const std::vector<double> T = {0.0, 2.0, 6.0};
  const std::vector<double> k = {10.0, 0.0, 40.0};

  EquilibriumConstantInterpolator logk(T, k, "piecewise-linear");
  EXPECT_EQ(logk.getSampleSize(), T.size());

  for (unsigned i = 0; i < 3; ++i)
    EXPECT_NEAR(logk.sample(T[i]), k[i], tol);
  EXPECT_NEAR(logk.sample(1.0), 5.0, tol);
  EXPECT_NEAR(logk.sample(3.0), 10.0, tol);
  EXPECT_NEAR(logk.sample(-1.0), 10.0, tol);
  EXPECT_NEAR(logk.sample(600), 40.0, tol);

  EXPECT_EQ(logk.sampleDerivative(-1.0), 0.0);
  EXPECT_NEAR(logk.sampleDerivative(1.0), -5.0, tol);
  EXPECT_NEAR(logk.sampleDerivative(3.0), 10.0, tol);
  EXPECT_EQ(logk.sampleDerivative(600.0), 0.0);

  // Check sampleDerivative() with the AD version produces the correct error
  DualReal T2 = 5.0;
  Moose::derivInsert(T2.derivatives(), 0, 1.0);
  try
  {
    EXPECT_NEAR(logk.sampleDerivative(5.0), logk.sample(T2).derivatives()[0], tol);
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find("Dual cannot be used for specified fit type in EquilibriumConstantInterpolator") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

/// Piecewise-linear with just one value
TEST(EquilibriumConstantInterpolatorTest, piecewiselinear_oneval)
{
  const std::vector<double> T = {2.0};
  const std::vector<double> k = {10.0};

  EquilibriumConstantInterpolator logk(T, k, "piecewise-linear");
  EXPECT_EQ(logk.getSampleSize(), T.size());

  EXPECT_EQ(logk.sample(1.0), 10.0);
  EXPECT_EQ(logk.sample(3.0), 10.0);

  EXPECT_EQ(logk.sampleDerivative(-1.0), 0.0);
  EXPECT_EQ(logk.sampleDerivative(600.0), 0.0);
}

/// Piecewise-linear with 500 values
TEST(EquilibriumConstantInterpolatorTest, piecewiselinear_500)
{
  const std::vector<double> T = {0.0, 2.0, 4.0, 6.0, 8.0};
  const std::vector<double> k = {10.0, 0.0, 500.0, 40.0, 500.0};

  EquilibriumConstantInterpolator logk(T, k, "piecewise-linear");
  EXPECT_EQ(logk.getSampleSize(), (std::size_t)3);

  logk.generate(); // does nothing relevant because type = piecewise-linear

  EXPECT_NEAR(logk.sample(T[0]), k[0], tol);
  EXPECT_NEAR(logk.sample(T[1]), k[1], tol);
  EXPECT_NEAR(logk.sample(T[3]), k[3], tol);
  EXPECT_NEAR(logk.sample(1.0), 5.0, tol);
  EXPECT_NEAR(logk.sample(3.0), 10.0, tol);
  EXPECT_NEAR(logk.sample(-1.0), 10.0, tol);
  EXPECT_NEAR(logk.sample(600), 40.0, tol);

  EXPECT_EQ(logk.sampleDerivative(-1.0), 0.0);
  EXPECT_NEAR(logk.sampleDerivative(1.0), -5.0, tol);
  EXPECT_NEAR(logk.sampleDerivative(3.0), 10.0, tol);
  EXPECT_EQ(logk.sampleDerivative(600.0), 0.0);
}

/// Piecewise-linear exception due to badly ordered T values
TEST(EquilibriumConstantInterpolatorTest, piecewiselinear_except)
{
  const std::vector<double> T = {2.0, 0.0, 6.0};
  const std::vector<double> k = {10.0, 0.0, 40.0};

  try
  {

    EquilibriumConstantInterpolator logk(T, k, "piecewise-linear");
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("EquilibriumConstantInterpolation: x-values are not strictly increasing: "
                         "x[0]: 2 x[1]: 0") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

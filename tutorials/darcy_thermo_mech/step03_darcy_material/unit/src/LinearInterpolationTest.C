/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

// MOOSE Includes
#include "LinearInterpolation.h"

// Google Test includes
#include "gtest/gtest.h"

TEST(LinearInterpolationTest, verifyLinearInterpolationObject)
{
  // Verify that the LinearInterpolation object in MOOSE works
  // correctly for the parameter range of interest in this problem.
  std::vector<double> x = {1, 3};
  std::vector<double> y = {0.8451e-9, 8.968e-9};
  LinearInterpolation interp(x, y);

  // Make sure the number of samples matches.
  ASSERT_EQ(interp.getSampleSize(), x.size());

  // The linear interpolation should match at the endpoints.
  EXPECT_DOUBLE_EQ(interp.sample(1.), 0.8451e-9);
  EXPECT_DOUBLE_EQ(interp.sample(3.), 8.968e-9);

  // Verify that the midpoint value is correct. This verification
  // value is something we computed independently and are checking
  // here.
  EXPECT_DOUBLE_EQ(interp.sample(2), 4.90655e-09);
}

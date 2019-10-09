#include "gtest/gtest.h"
#include "Numerics.h"
#include "THMTestUtils.h"

TEST(NumericsTest, test_absoluteFuzzyEqualVectors_True)
{
  const RealVectorValue a(1.0, 2.0, 3.0);
  const RealVectorValue b(1.0 + 1.0e-15, 2.0, 3.0);
  EXPECT_TRUE(THM::absoluteFuzzyEqualVectors(a, b));
}

TEST(NumericsTest, test_absoluteFuzzyEqualVectors_False)
{
  const RealVectorValue a(1.0, 2.0, 3.0);
  const RealVectorValue b(1.1, 2.0, 3.0);
  EXPECT_FALSE(THM::absoluteFuzzyEqualVectors(a, b));
}

TEST(NumericsTest, test_areParallelVectors_True)
{
  const RealVectorValue a(1.0, 2.0, 3.0);
  const RealVectorValue b(-2.0, -4.0, -6.0);
  EXPECT_TRUE(THM::areParallelVectors(a, b));
}

TEST(NumericsTest, test_areParallelVectors_False)
{
  const RealVectorValue a(1.0, 2.0, 3.0);
  const RealVectorValue b(2.0, 3.0, 1.0);
  EXPECT_FALSE(THM::areParallelVectors(a, b));
}

TEST(NumericsTest, test_haveSameDirection_True)
{
  const RealVectorValue a(1.0, 2.0, 3.0);
  const RealVectorValue b(2.0, 4.0, 6.0);
  EXPECT_TRUE(THM::haveSameDirection(a, b));
}

TEST(NumericsTest, test_haveSameDirection_False)
{
  const RealVectorValue a(1.0, 2.0, 3.0);
  const RealVectorValue b(-1.0, 2.0, 3.0);
  EXPECT_FALSE(THM::haveSameDirection(a, b));
}

TEST(NumericsTest, Reynolds)
{
  ABS_TEST(THM::Reynolds(0.1, 999, 0.5, 2e-2, 0.9), 1.11, 1e-13);
  ABS_TEST(THM::Reynolds(0.1, 999, -0.5, 2e-2, 0.9), 1.11, 1e-13);
}

TEST(NumericsTest, Prandtl) { ABS_TEST(THM::Prandtl(10, 0.1, 2), 0.5, 1e-13); }

TEST(NumericsTest, Grashof)
{
  ABS_TEST(THM::Grashof(0.1, 1, 2e-2, 999, 0.05, 9.81), 3.1329247392e3, 1e-13);
}

TEST(NumericsTest, Laplace) { ABS_TEST(THM::Laplace(0.001, 1, 9.81), 0.010096375546923, 1e-13); }

TEST(NumericsTest, viscosityNumber)
{
  ABS_TEST(THM::viscosityNumber(0.05, 0.02, 999, 2, 9.81), 0.062602188259, 1e-13);
}

TEST(NumericsTest, wallHeatTransferCoefficient)
{
  ABS_TEST(THM::wallHeatTransferCoefficient(2, 6, 3), 4, 1e-13);
}

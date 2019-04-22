#include "NumericsTest.h"
#include "Numerics.h"

TEST_F(NumericsTest, test_absoluteFuzzyEqualVectors_True)
{
  const RealVectorValue a(1.0, 2.0, 3.0);
  const RealVectorValue b(1.0 + 1.0e-15, 2.0, 3.0);
  EXPECT_TRUE(THM::absoluteFuzzyEqualVectors(a, b));
}

TEST_F(NumericsTest, test_absoluteFuzzyEqualVectors_False)
{
  const RealVectorValue a(1.0, 2.0, 3.0);
  const RealVectorValue b(1.1, 2.0, 3.0);
  EXPECT_FALSE(THM::absoluteFuzzyEqualVectors(a, b));
}

TEST_F(NumericsTest, test_areParallelVectors_True)
{
  const RealVectorValue a(1.0, 2.0, 3.0);
  const RealVectorValue b(-2.0, -4.0, -6.0);
  EXPECT_TRUE(THM::areParallelVectors(a, b));
}

TEST_F(NumericsTest, test_areParallelVectors_False)
{
  const RealVectorValue a(1.0, 2.0, 3.0);
  const RealVectorValue b(2.0, 3.0, 1.0);
  EXPECT_FALSE(THM::areParallelVectors(a, b));
}

TEST_F(NumericsTest, test_haveSameDirection_True)
{
  const RealVectorValue a(1.0, 2.0, 3.0);
  const RealVectorValue b(2.0, 4.0, 6.0);
  EXPECT_TRUE(THM::haveSameDirection(a, b));
}

TEST_F(NumericsTest, test_haveSameDirection_False)
{
  const RealVectorValue a(1.0, 2.0, 3.0);
  const RealVectorValue b(-1.0, 2.0, 3.0);
  EXPECT_FALSE(THM::haveSameDirection(a, b));
}

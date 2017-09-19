
#include "gtest/gtest.h"

#include "MooseUtils.h"

struct TestCase
{
  std::string a;
  std::string b;
  int dist;
};

TEST(MooseUtilsTests, levenshteinDist)
{
  TestCase cases[] = {
      {"hello", "hell", 1}, {"flood", "foods", 2}, {"fandango", "odanget", 5},
  };

  for (size_t i = 0; i < sizeof(cases) / sizeof(TestCase); i++)
  {
    auto test = cases[i];
    int got = MooseUtils::levenshteinDist(test.a, test.b);
    EXPECT_EQ(test.dist, got) << "case " << i + 1 << " FAILED: a=" << test.a << ", b=" << test.b;
  }
}

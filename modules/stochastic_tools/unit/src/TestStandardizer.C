#include "gtest/gtest.h"
#include <torch/torch.h>
#include "Standardizer.h"

using namespace StochasticTools;

TEST(StochasticTools, computeSet)
{
  const std::vector<double> mean_gold = {0.0, -1.0};
  const std::vector<double> stddev_gold = {1.0, 0.0};

  RealEigenMatrix input_matrix{
      {1.0, -1.0}, {1.0, -1.0}, {1.0, -1.0}, {-1.0, -1.0}, {-1.0, -1.0}, {-1.0, -1.0}};

  StochasticTools::Standardizer eigen;
  eigen.computeSet(input_matrix);

  auto mean = eigen.getMean();
  auto stddev = eigen.getStdDev();
  unsigned int n = mean_gold.size();
  for (unsigned int i = 0; i < n; ++i)
  {
    EXPECT_EQ(mean.data()[i], mean_gold[i]);
    EXPECT_EQ(stddev.data()[i], stddev_gold[i]);
  }
}

TEST(StochasticTools, getStandardized)
{
  StochasticTools::Standardizer eigen;
  RealEigenMatrix input{{1.0, -1.0}, {-1.0, 1.0}};
  RealEigenMatrix gold{{1.0, -1.0}, {-1.0, 1.0}};
  eigen.computeSet(input);
  eigen.getStandardized(input);
  for (unsigned i = 0; i < input.size(); i++)
    EXPECT_EQ(input.data()[i], gold.data()[i]);
}

TEST(StochasticTools, getDestandardized)
{
  StochasticTools::Standardizer eigen;
  RealEigenMatrix input{{1.0, -1.0}, {-1.0, 1.0}};
  RealEigenMatrix gold{{1.0, -1.0}, {-1.0, 1.0}};
  eigen.computeSet(input);
  eigen.getDestandardized(input);
  for (unsigned i = 0; i < input.size(); i++)
    EXPECT_EQ(input.data()[i], gold.data()[i]);
}

TEST(StochasticTools, getDescaled)
{
  // input = input.array().rowwise() * stdev.transpose().array();
  StochasticTools::Standardizer eigen;
  RealEigenMatrix input{{1.0, -1.0}, {1.0, 1.0}};
  RealEigenMatrix gold{{0.0, -1.0}, {0.0, 1.0}};
  eigen.computeSet(input);
  eigen.getDescaled(input);
  for (unsigned i = 0; i < input.size(); i++)
    EXPECT_EQ(input.data()[i], gold.data()[i]);
}

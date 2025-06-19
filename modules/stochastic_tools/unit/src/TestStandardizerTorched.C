#ifdef LIBTORCH_ENABLED

#include "gtest/gtest.h"
#include <torch/torch.h>
#include "Standardizer.h"

using namespace StochasticTools;

TEST(StochasticTools, getMean)
{
  const std::vector<double> mean_gold = {0.0, -1.0};
  const std::vector<double> stddev_gold = {1.0, 0.0};

  torch::Tensor input_tensor = torch::tensor(
      {{1.0, -1.0}, {-1.0, -1.0}, {1.0, -1.0}, {-1.0, -1.0}, {1.0, -1.0}, {-1.0, -1.0}},
      {torch::kFloat64});

  StochasticTools::Standardizer torch;
  torch.computeSet(input_tensor);

  auto mean = torch.getMean();
  auto stddev = torch.getStdDev();
  unsigned int n = mean.sizes()[1];
  for (unsigned int i = 0; i < n; ++i)
  {
    EXPECT_EQ(mean[i].item<Real>(), mean_gold[i]);
    EXPECT_EQ(stddev.data_ptr<Real>()[i], stddev_gold[i]);
  }
}

TEST(StochasticTools, getStandardized)
{
  const std::vector<double> gold = {1.0, -1.0, -1.0, 1.0};

  torch::Tensor input_tensor = torch::tensor({{1.0, -1.0}, {-1.0, 1.0}}, {torch::kFloat64});

  StochasticTools::Standardizer torch;
  torch.computeSet(input_tensor);
  torch.getStandardized(input_tensor);

  for (unsigned int i = 0; i < gold.size(); ++i)
  {
    EXPECT_EQ(input_tensor.data_ptr<Real>()[i], gold[i]);
  }
}

TEST(StochasticTools, getDestandardized)
{
  const std::vector<double> gold = {1.0, -1.0, -1.0, 1.0};

  torch::Tensor input_tensor = torch::tensor({{1.0, -1.0}, {-1.0, 1.0}}, {torch::kFloat64});

  StochasticTools::Standardizer torch;
  torch.computeSet(input_tensor);
  torch.getStandardized(input_tensor);
  torch.getDestandardized(input_tensor);
  for (unsigned int i = 0; i < gold.size(); ++i)
  {
    EXPECT_EQ(input_tensor.data_ptr<Real>()[i], gold[i]);
  }
}

TEST(StochasticTools, getDescaled)
{
  // input = input.array().rowwise() * stdev.transpose().array();
  StochasticTools::Standardizer torch;
  torch::Tensor input_tensor = torch::tensor({{1.0, -1.0}, {1.0, 1.0}}, {torch::kFloat64});
  const std::vector<double> gold = {0.0, -1.0, 0.0, 1.0};

  torch.computeSet(input_tensor);
  torch.getDescaled(input_tensor);
  for (unsigned int i = 0; i < gold.size(); ++i)
  {
    EXPECT_EQ(input_tensor.data_ptr<Real>()[i], gold[i]);
  }
}

#endif

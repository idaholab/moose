#ifdef LIBTORCH_ENABLED

#include "gtest/gtest.h"
#include <torch/torch.h>
#include "StandardizerTorched.h"

using namespace StochasticToolsTorched;

TEST(StochasticToolsTorched, computeSet)
{
  const std::vector<double> mean_gold = {0.0, -1.0};

  torch::Tensor input_tensor = torch::tensor(
      {{1.0, -1.0}, {1.0, -1.0}, {1.0, -1.0}, {-1.0, -1.0}, {-1.0, -1.0}, {-1.0, -1.0}},
      {torch::kFloat64});

  StochasticToolsTorched::StandardizerTorched torch;
  torch.computeSet(input_tensor);

  auto mean = torch.getMean();
  auto stddev = torch.getStdDev();
  unsigned int n = mean.size(-1);
  for (unsigned int i = 0; i < n; ++i)
  {
    EXPECT_EQ(mean[i].item<Real>(), mean_gold[i]);
  }
}

TEST(StochasticToolsTorched, getStandardized) {}

TEST(StochasticToolsTorched, getDestandardized) {}

TEST(StochasticToolsTorched, getDescaled) {}

#endif
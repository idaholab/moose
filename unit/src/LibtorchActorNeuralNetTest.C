//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_LIBTORCH_ENABLED

#include "gtest/gtest.h"
#include "LibtorchActorNeuralNet.h"

#include <cmath>

namespace
{

class TestableLibtorchArtificialNeuralNet : public Moose::LibtorchArtificialNeuralNet
{
public:
  using Moose::LibtorchArtificialNeuralNet::_weights;
  using Moose::LibtorchArtificialNeuralNet::LibtorchArtificialNeuralNet;
};

class TestableLibtorchActorNeuralNet : public Moose::LibtorchActorNeuralNet
{
public:
  using Moose::LibtorchActorNeuralNet::_weights;
  using Moose::LibtorchActorNeuralNet::LibtorchActorNeuralNet;
};

Real
inverseSoftplusPlusOne(const Real target)
{
  return std::log(std::exp(target - 1.0) - 1.0);
}

} // namespace

TEST(LibtorchActorNeuralNetTest, artificialNetAppliesAffineInputAndOutputScaling)
{
  TestableLibtorchArtificialNeuralNet network("test_ann",
                                              2,
                                              1,
                                              {},
                                              {"linear"},
                                              {},
                                              {},
                                              torch::kCPU,
                                              torch::kDouble,
                                              true,
                                              {1.0, 2.0},
                                              {2.0, 3.0},
                                              {10.0});

  ASSERT_EQ(network._weights.size(), 1);

  network._weights[0]->weight.data().fill_(0.0);
  network._weights[0]->weight.data()[0][0] = 1.0;
  network._weights[0]->weight.data()[0][1] = -1.0;
  network._weights[0]->bias.data().fill_(0.0);

  auto input = torch::tensor({{2.0, 6.0}}, at::kDouble);
  const Real actual = network.forward(input).item<Real>();

  EXPECT_NEAR(actual, -100.0, 1e-12);
}

TEST(LibtorchActorNeuralNetTest, boundedBetaLogProbability)
{
  constexpr Real min_value = -2.0;
  constexpr Real max_value = 4.0;
  constexpr Real alpha_target = 2.3;
  constexpr Real beta_target = 3.7;
  constexpr Real action_value = 1.2;

  TestableLibtorchActorNeuralNet network(
      "test_beta", 1, 1, {1}, {"linear"}, {min_value}, {max_value});

  ASSERT_EQ(network._weights.size(), 1);

  network._weights[0]->weight.data().fill_(0.0);
  network._weights[0]->bias.data().fill_(1.0);
  network.actionDistributionHead().primaryModule()->weight.data().fill_(
      inverseSoftplusPlusOne(alpha_target));
  network.actionDistributionHead().secondaryModule()->weight.data().fill_(
      inverseSoftplusPlusOne(beta_target));

  auto input = torch::zeros({1, 1}, at::kDouble);
  network.evaluate(input, false);

  const Real alpha = network.alphaTensor().item<Real>();
  const Real beta = network.betaTensor().item<Real>();
  const Real normalized = (action_value - min_value) / (max_value - min_value);
  const Real log_norm = std::lgamma(alpha) + std::lgamma(beta) - std::lgamma(alpha + beta);
  const Real expected = (alpha - 1.0) * std::log(normalized) +
                        (beta - 1.0) * std::log1p(-normalized) - log_norm -
                        std::log(max_value - min_value);

  auto action = torch::tensor({{action_value}}, at::kDouble);
  const Real actual = network.logProbability(action).item<Real>();

  EXPECT_NEAR(actual, expected, 1e-12);
}

TEST(LibtorchActorNeuralNetTest, gaussianActorUsesPhysicalActionScaling)
{
  constexpr Real input_shift = 1.0;
  constexpr Real input_scale = 2.0;
  constexpr Real action_scale = 5.0;
  const Real log_std = std::log(2.0);
  constexpr Real physical_action = 20.0;
  constexpr Real expected_deterministic_action = 15.0;

  TestableLibtorchActorNeuralNet network("test_gaussian",
                                         1,
                                         1,
                                         {},
                                         {"linear"},
                                         {},
                                         {},
                                         torch::kCPU,
                                         torch::kDouble,
                                         true,
                                         {input_shift},
                                         {input_scale},
                                         {action_scale});

  network.actionDistributionHead().primaryModule()->weight.data().fill_(1.5);
  network.actionDistributionHead().primaryModule()->bias.data().fill_(0.0);
  network.actionDistributionHead().secondaryModule()->weight.data().fill_(log_std / 2.0);
  network.actionDistributionHead().secondaryModule()->bias.data().fill_(0.0);

  auto input = torch::tensor({{2.0}}, at::kDouble);
  const Real deterministic_action = network.evaluate(input, false).item<Real>();
  EXPECT_NEAR(deterministic_action, expected_deterministic_action, 1e-12);

  const Real unscaled_mean = expected_deterministic_action / action_scale;
  const Real unscaled_action = physical_action / action_scale;
  constexpr Real pi = 3.14159265358979323846;
  const Real expected_log_probability =
      -std::pow(unscaled_action - unscaled_mean, 2) / (2.0 * 4.0) - log_std -
      0.5 * std::log(2.0 * pi) - std::log(action_scale);

  auto action = torch::tensor({{physical_action}}, at::kDouble);
  const Real actual_log_probability = network.logProbability(action).item<Real>();

  EXPECT_NEAR(actual_log_probability, expected_log_probability, 1e-12);
}

#endif

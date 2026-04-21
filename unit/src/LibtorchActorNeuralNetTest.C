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

class TestableLibtorchActorNeuralNet : public Moose::LibtorchActorNeuralNet
{
public:
  using Moose::LibtorchActorNeuralNet::_alpha_module;
  using Moose::LibtorchActorNeuralNet::_beta_module;
  using Moose::LibtorchActorNeuralNet::_weights;
  using Moose::LibtorchActorNeuralNet::LibtorchActorNeuralNet;
};

Real
inverseSoftplusPlusOne(const Real target)
{
  return std::log(std::exp(target - 1.0) - 1.0);
}

} // namespace

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
  ASSERT_EQ(network._alpha_module.size(), 1);
  ASSERT_EQ(network._beta_module.size(), 1);

  network._weights[0]->weight.data().fill_(0.0);
  network._weights[0]->bias.data().fill_(1.0);
  network._alpha_module[0]->weight.data().fill_(inverseSoftplusPlusOne(alpha_target));
  network._beta_module[0]->weight.data().fill_(inverseSoftplusPlusOne(beta_target));

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

#endif

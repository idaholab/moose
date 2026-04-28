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
#include "LibtorchRandomUtils.h"
#include "MooseUnitUtils.h"

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
                                              torch::kCPU,
                                              torch::kDouble,
                                              true,
                                              {1.0, 2.0},
                                              {2.0, 3.0},
                                              {10.0});

  ASSERT_EQ(network._weights.size(), 1u);

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

  ASSERT_EQ(network._weights.size(), 1u);

  network._weights[0]->weight.data().fill_(0.0);
  network._weights[0]->bias.data().fill_(1.0);
  network.betaActionDistribution().alphaModule()->weight.data().fill_(
      inverseSoftplusPlusOne(alpha_target));
  network.betaActionDistribution().betaModule()->weight.data().fill_(
      inverseSoftplusPlusOne(beta_target));

  auto input = torch::zeros({1, 1}, at::kDouble);
  network.evaluate(input, false);

  const Real alpha = network.betaActionDistribution().alphaTensor().item<Real>();
  const Real beta = network.betaActionDistribution().betaTensor().item<Real>();
  const Real normalized = (action_value - min_value) / (max_value - min_value);
  const Real log_norm = std::lgamma(alpha) + std::lgamma(beta) - std::lgamma(alpha + beta);
  const Real expected = (alpha - 1.0) * std::log(normalized) +
                        (beta - 1.0) * std::log1p(-normalized) - log_norm -
                        std::log(max_value - min_value);

  auto action = torch::tensor({{action_value}}, at::kDouble);
  const Real actual = network.logProbability(action).item<Real>();

  EXPECT_NEAR(actual, expected, 1e-12);
}

TEST(LibtorchActorNeuralNetTest, gaussianActorUsesPhysicalActionScalingAndStateIndependentStd)
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

  network.gaussianActionDistribution().meanModule()->weight.data().fill_(1.5);
  network.gaussianActionDistribution().meanModule()->bias.data().fill_(0.0);
  network.gaussianActionDistribution().stdModule()->weight.data().fill_(123.0);
  network.gaussianActionDistribution().stdModule()->bias.data().fill_(log_std);

  auto input = torch::tensor({{2.0}}, at::kDouble);
  const Real deterministic_action = network.evaluate(input, false).item<Real>();
  EXPECT_NEAR(deterministic_action, expected_deterministic_action, 1e-12);
  EXPECT_NEAR(
      network.gaussianActionDistribution().stdTensor().item<Real>(), std::exp(log_std), 1e-12);

  const Real unscaled_mean = expected_deterministic_action / action_scale;
  const Real unscaled_action = physical_action / action_scale;
  const Real expected_log_probability =
      -std::pow(unscaled_action - unscaled_mean, 2) / (2.0 * 4.0) - log_std -
      0.5 * std::log(2.0 * libMesh::pi) - std::log(action_scale);

  auto action = torch::tensor({{physical_action}}, at::kDouble);
  const Real actual_log_probability = network.logProbability(action).item<Real>();

  EXPECT_NEAR(actual_log_probability, expected_log_probability, 1e-12);

  auto second_input = torch::tensor({{4.0}}, at::kDouble);
  network.evaluate(second_input, false);
  EXPECT_NEAR(
      network.gaussianActionDistribution().stdTensor().item<Real>(), std::exp(log_std), 1e-12);
}

TEST(LibtorchActorNeuralNetTest, gaussianActorCanUseStateDependentStdWhenRequested)
{
  TestableLibtorchActorNeuralNet network("test_state_dependent_gaussian",
                                         1,
                                         1,
                                         {},
                                         {"linear"},
                                         {},
                                         {},
                                         torch::kCPU,
                                         torch::kDouble,
                                         true,
                                         {1.0},
                                         {2.0},
                                         {1.0},
                                         false);

  network.gaussianActionDistribution().meanModule()->weight.data().fill_(0.0);
  network.gaussianActionDistribution().meanModule()->bias.data().fill_(0.0);
  network.gaussianActionDistribution().stdModule()->weight.data().fill_(0.5);
  network.gaussianActionDistribution().stdModule()->bias.data().fill_(0.0);

  auto first_input = torch::tensor({{2.0}}, at::kDouble);
  network.evaluate(first_input, false);
  const Real first_std = network.gaussianActionDistribution().stdTensor().item<Real>();

  auto second_input = torch::tensor({{4.0}}, at::kDouble);
  network.evaluate(second_input, false);
  const Real second_std = network.gaussianActionDistribution().stdTensor().item<Real>();

  EXPECT_NEAR(first_std, std::exp(1.0), 1e-12);
  EXPECT_NEAR(second_std, std::exp(3.0), 1e-12);
  EXPECT_GT(second_std, first_std);
}

TEST(LibtorchActorNeuralNetTest, explicitGeneratorKeepsGaussianSamplingStableAcrossCopies)
{
  TestableLibtorchActorNeuralNet original("test_gaussian",
                                          1,
                                          1,
                                          {},
                                          {"linear"},
                                          {},
                                          {},
                                          torch::kCPU,
                                          torch::kDouble,
                                          true,
                                          {0.0},
                                          {1.0},
                                          {1.0});

  original.gaussianActionDistribution().meanModule()->weight.data().fill_(0.75);
  original.gaussianActionDistribution().meanModule()->bias.data().fill_(1.25);
  original.gaussianActionDistribution().stdModule()->weight.data().fill_(0.0);
  original.gaussianActionDistribution().stdModule()->bias.data().fill_(std::log(0.5));

  TestableLibtorchActorNeuralNet copied(original);

  auto original_input = torch::tensor({{2.0}}, at::kDouble);
  auto copied_input = torch::tensor({{2.0}}, at::kDouble);
  auto original_generator = Moose::makeLibtorchCPUGenerator(12345);
  auto copied_generator = Moose::makeLibtorchCPUGenerator(12345);

  const auto original_action = original.evaluate(original_input, true, original_generator);
  const auto copied_action = copied.evaluate(copied_input, true, copied_generator);

  EXPECT_TRUE(torch::allclose(original_action, copied_action, /* rtol = */ 0.0, /* atol = */ 0.0));
}

TEST(LibtorchActorNeuralNetTest, explicitGeneratorMakesInitializationIndependentOfConstructionOrder)
{
  TestableLibtorchActorNeuralNet first("first_actor",
                                       2,
                                       1,
                                       {3},
                                       {"relu"},
                                       {},
                                       {},
                                       torch::kCPU,
                                       torch::kDouble,
                                       true,
                                       {0.0, 0.0},
                                       {1.0, 1.0},
                                       {1.0});
  TestableLibtorchActorNeuralNet second("second_actor",
                                        2,
                                        1,
                                        {3},
                                        {"relu"},
                                        {},
                                        {},
                                        torch::kCPU,
                                        torch::kDouble,
                                        true,
                                        {0.0, 0.0},
                                        {1.0, 1.0},
                                        {1.0});

  first.initializeNeuralNetwork(Moose::makeLibtorchCPUGenerator(2468));
  second.initializeNeuralNetwork(Moose::makeLibtorchCPUGenerator(2468));

  const auto first_parameters = first.named_parameters();
  const auto second_parameters = second.named_parameters();
  ASSERT_EQ(first_parameters.size(), second_parameters.size());
  for (const auto i : index_range(first_parameters))
  {
    EXPECT_EQ(first_parameters[i].key(), second_parameters[i].key());
    EXPECT_TRUE(torch::allclose(first_parameters[i].value(),
                                second_parameters[i].value(),
                                /* rtol = */ 0.0,
                                /* atol = */ 0.0));
  }
}

TEST(LibtorchActorNeuralNetTest, loadActorStateAcceptsTorchSaveArchive)
{
  TestableLibtorchActorNeuralNet saved("saved_actor",
                                       2,
                                       1,
                                       {2},
                                       {"linear"},
                                       {},
                                       {},
                                       torch::kCPU,
                                       torch::kDouble,
                                       true,
                                       {1.0, -2.0},
                                       {0.5, 3.0},
                                       {4.0});

  saved._weights[0]->weight.data() = torch::tensor({{1.0, 2.0}, {3.0, 4.0}}, at::kDouble);
  saved._weights[0]->bias.data() = torch::tensor({5.0, 6.0}, at::kDouble);
  saved.gaussianActionDistribution().meanModule()->weight.data() =
      torch::tensor({{7.0, 8.0}}, at::kDouble);
  saved.gaussianActionDistribution().meanModule()->bias.data() = torch::tensor({9.0}, at::kDouble);
  saved.gaussianActionDistribution().stdModule()->weight.data() =
      torch::tensor({{-1.5, 2.5}}, at::kDouble);
  saved.gaussianActionDistribution().stdModule()->bias.data() = torch::tensor({-3.5}, at::kDouble);

  Moose::UnitUtils::TempFile archive;
  torch::save(std::make_shared<Moose::LibtorchActorNeuralNet>(saved), archive.path().string());

  TestableLibtorchActorNeuralNet restored("restored_actor",
                                          2,
                                          1,
                                          {2},
                                          {"linear"},
                                          {},
                                          {},
                                          torch::kCPU,
                                          torch::kDouble,
                                          true,
                                          {1.0, -2.0},
                                          {0.5, 3.0},
                                          {4.0});

  Moose::loadLibtorchActorNeuralNetState(restored, archive.path().string());

  const auto saved_parameters = saved.named_parameters();
  const auto restored_parameters = restored.named_parameters();
  ASSERT_EQ(saved_parameters.size(), restored_parameters.size());
  for (std::size_t i = 0; i < saved_parameters.size(); ++i)
  {
    EXPECT_EQ(saved_parameters[i].key(), restored_parameters[i].key());
    EXPECT_TRUE(torch::allclose(saved_parameters[i].value(),
                                restored_parameters[i].value(),
                                /*rtol=*/0.0,
                                /*atol=*/0.0));
  }

  const auto saved_buffers = saved.named_buffers();
  const auto restored_buffers = restored.named_buffers();
  ASSERT_EQ(saved_buffers.size(), restored_buffers.size());
  for (std::size_t i = 0; i < saved_buffers.size(); ++i)
  {
    EXPECT_EQ(saved_buffers[i].key(), restored_buffers[i].key());
    EXPECT_TRUE(torch::allclose(saved_buffers[i].value(),
                                restored_buffers[i].value(),
                                /*rtol=*/0.0,
                                /*atol=*/0.0));
  }

  auto saved_input = torch::tensor({{3.0, -1.0}}, at::kDouble);
  auto restored_input = saved_input.clone();
  EXPECT_TRUE(torch::allclose(saved.evaluate(saved_input, false),
                              restored.evaluate(restored_input, false),
                              /*rtol=*/0.0,
                              /*atol=*/0.0));
}

TEST(LibtorchActorNeuralNetTest, loadActorStateRejectsHiddenLayerMismatch)
{
  TestableLibtorchActorNeuralNet saved("saved_actor",
                                       2,
                                       1,
                                       {2},
                                       {"linear"},
                                       {},
                                       {},
                                       torch::kCPU,
                                       torch::kDouble,
                                       true,
                                       {1.0, -2.0},
                                       {0.5, 3.0},
                                       {4.0});

  Moose::UnitUtils::TempFile archive;
  torch::save(std::make_shared<Moose::LibtorchActorNeuralNet>(saved), archive.path().string());

  TestableLibtorchActorNeuralNet restored("restored_actor",
                                          2,
                                          1,
                                          {3},
                                          {"linear"},
                                          {},
                                          {},
                                          torch::kCPU,
                                          torch::kDouble,
                                          true,
                                          {1.0, -2.0},
                                          {0.5, 3.0},
                                          {4.0});

  EXPECT_ANY_THROW(Moose::loadLibtorchActorNeuralNetState(restored, archive.path().string()));
}

TEST(LibtorchActorNeuralNetTest, loadActorStateRejectsBoundednessMismatch)
{
  TestableLibtorchActorNeuralNet saved("saved_actor",
                                       1,
                                       1,
                                       {2},
                                       {"linear"},
                                       {},
                                       {},
                                       torch::kCPU,
                                       torch::kDouble,
                                       true,
                                       {1.0},
                                       {2.0},
                                       {1.0});

  Moose::UnitUtils::TempFile archive;
  torch::save(std::make_shared<Moose::LibtorchActorNeuralNet>(saved), archive.path().string());

  TestableLibtorchActorNeuralNet restored("restored_actor",
                                          1,
                                          1,
                                          {2},
                                          {"linear"},
                                          {-2.0},
                                          {2.0},
                                          torch::kCPU,
                                          torch::kDouble,
                                          true,
                                          {1.0},
                                          {2.0},
                                          {1.0});

  EXPECT_ANY_THROW(Moose::loadLibtorchActorNeuralNetState(restored, archive.path().string()));
}

#endif

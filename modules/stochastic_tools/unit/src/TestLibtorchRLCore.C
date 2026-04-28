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
#include "LibtorchArtificialNeuralNet.h"
#include "LibtorchObservationHistoryHelper.h"
#include "LibtorchRandomUtils.h"
#include "LibtorchRLMiniBatchSampler.h"
#include "LibtorchRLPPOLoss.h"
#include "LibtorchRLTrajectoryBuffer.h"
#include "LibtorchRLValueEstimator.h"

#include <cmath>

namespace
{

TEST(LibtorchRLCoreTest, ObservationHistoryStacksCurrentAndTrajectoryData)
{
  LibtorchObservationHistoryHelper history(3);

  const auto expanded_observation_factors = history.expandObservationFactors({0.5, 2.0});
  EXPECT_EQ(expanded_observation_factors, std::vector<Real>({0.5, 2.0, 0.5, 2.0, 0.5, 2.0}));

  std::vector<std::vector<Real>> old_observations;
  history.initializeHistory({1.0, 6.0}, old_observations);

  const auto stacked_current = history.stackCurrentObservation({3.0, 2.0}, old_observations);
  EXPECT_EQ(stacked_current, std::vector<Real>({3.0, 2.0, 1.0, 6.0, 1.0, 6.0}));

  std::vector<std::vector<Real>> observation_trajectories = {{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}};
  const auto stacked_trajectory = history.stackTrajectoryObservation(observation_trajectories, 2);
  EXPECT_EQ(stacked_trajectory, std::vector<Real>({3.0, 6.0, 2.0, 5.0, 1.0, 4.0}));
}

TEST(LibtorchRLCoreTest, ObservationHistoryRejectsMalformedStoredHistory)
{
  LibtorchObservationHistoryHelper history(3);

  EXPECT_THROW(
      {
        try
        {
          history.stackCurrentObservation({3.0, 2.0}, {{1.0, 6.0}});
        }
        catch (const std::exception & e)
        {
          EXPECT_EQ(std::string(e.what()),
                    "Observation history must contain 2 stored entries, but got 1.");
          throw;
        }
      },
      std::exception);
}

TEST(LibtorchRLCoreTest, ObservationHistoryRejectsZeroTimesteps)
{
  EXPECT_THROW(
      {
        try
        {
          LibtorchObservationHistoryHelper history(0);
          static_cast<void>(history);
        }
        catch (const std::exception & e)
        {
          EXPECT_EQ(std::string(e.what()),
                    "Observation history requires at least one input timestep.");
          throw;
        }
      },
      std::exception);
}

TEST(LibtorchRLCoreTest, ValueEstimatorComputesGAETargets)
{
  Moose::LibtorchArtificialNeuralNet value_network("value", 1, 1, {}, {"linear"});
  auto value_params = value_network.named_parameters();
  value_params[0].value().data().fill_(1.0);
  value_params[1].value().data().fill_(0.0);

  LibtorchRLTrajectoryBuffer::Trajectory trajectory;
  trajectory.observations = {{1.0}, {2.0}};
  trajectory.next_observations = {{2.0}, {3.0}};
  trajectory.rewards = {0.5, 1.0};

  LibtorchRLValueEstimator estimator(0.9, 0.95);
  const auto targets = estimator.estimate(trajectory, value_network);

  ASSERT_EQ(targets.advantages.size(), 2u);
  ASSERT_EQ(targets.value_targets.size(), 2u);
  EXPECT_NEAR(targets.advantages[0], 2.7535, 1e-12);
  EXPECT_NEAR(targets.advantages[1], 1.7, 1e-12);
  EXPECT_NEAR(targets.value_targets[0], 3.7535, 1e-12);
  EXPECT_NEAR(targets.value_targets[1], 3.7, 1e-12);
}

TEST(LibtorchRLCoreTest, PPOLossUsesStoredLogProbabilityAndValueTarget)
{
  constexpr Real pi = 3.14159265358979323846;

  Moose::LibtorchActorNeuralNet policy_network("policy", 1, 1, {}, {"linear"});
  policy_network.gaussianActionDistribution().meanModule()->weight.data().fill_(0.0);
  policy_network.gaussianActionDistribution().meanModule()->bias.data().fill_(0.0);
  policy_network.gaussianActionDistribution().stdModule()->weight.data().fill_(0.0);
  policy_network.gaussianActionDistribution().stdModule()->bias.data().fill_(0.0);

  Moose::LibtorchArtificialNeuralNet value_network("value", 1, 1, {}, {"linear"});
  auto value_params = value_network.named_parameters();
  value_params[0].value().data().fill_(0.0);
  value_params[1].value().data().fill_(1.0);

  LibtorchRLMiniBatch batch;
  batch.observations = torch::zeros({1, 1}, torch::TensorOptions().dtype(torch::kDouble));
  batch.actions = torch::zeros({1, 1}, torch::TensorOptions().dtype(torch::kDouble));
  batch.old_log_probabilities =
      torch::tensor({{-0.5 * std::log(2.0 * pi)}}, torch::TensorOptions().dtype(torch::kDouble));
  batch.value_targets = torch::tensor({{1.5}}, torch::TensorOptions().dtype(torch::kDouble));
  batch.advantages = torch::tensor({{2.0}}, torch::TensorOptions().dtype(torch::kDouble));

  LibtorchRLPPOLoss loss(0.2, 0.01);
  const auto loss_values = loss.compute(policy_network, value_network, batch);

  const Real expected_entropy = 0.5 * std::log(2.0 * pi) + 0.5;
  const Real expected_actor_loss = -(2.0 + 0.01 * expected_entropy);
  const Real expected_critic_loss = 0.25;

  EXPECT_NEAR(loss_values.entropy.item<Real>(), expected_entropy, 1e-12);
  EXPECT_NEAR(loss_values.actor_loss.item<Real>(), expected_actor_loss, 1e-12);
  EXPECT_NEAR(loss_values.critic_loss.item<Real>(), expected_critic_loss, 1e-12);
}

TEST(LibtorchRLCoreTest, MiniBatchSamplerStandardizesAdvantagesPerBatch)
{
  LibtorchRLTrajectoryBuffer::TensorBatch batch;
  batch.observations =
      torch::tensor({{0.0}, {1.0}, {2.0}, {3.0}}, torch::TensorOptions().dtype(torch::kDouble));
  batch.actions =
      torch::tensor({{0.1}, {0.2}, {0.3}, {0.4}}, torch::TensorOptions().dtype(torch::kDouble));
  batch.log_probabilities =
      torch::tensor({{-1.0}, {-1.1}, {-1.2}, {-1.3}}, torch::TensorOptions().dtype(torch::kDouble));
  batch.value_targets =
      torch::tensor({{1.0}, {2.0}, {3.0}, {4.0}}, torch::TensorOptions().dtype(torch::kDouble));
  batch.advantages =
      torch::tensor({{1.0}, {2.0}, {3.0}, {4.0}}, torch::TensorOptions().dtype(torch::kDouble));

  LibtorchRLMiniBatchSampler sampler;
  const auto mini_batches = sampler.sample(batch, 2, true);

  ASSERT_EQ(mini_batches.size(), 2u);
  for (const auto & mini_batch : mini_batches)
  {
    ASSERT_EQ(mini_batch.size(), 2);
    EXPECT_NEAR(mini_batch.advantages.mean().item<Real>(), 0.0, 1e-12);
  }
}

TEST(LibtorchRLCoreTest, MiniBatchSamplerUsesExplicitGeneratorForDeterministicShuffling)
{
  LibtorchRLTrajectoryBuffer::TensorBatch batch;
  batch.observations =
      torch::tensor({{0.0}, {1.0}, {2.0}, {3.0}}, torch::TensorOptions().dtype(torch::kDouble));
  batch.actions =
      torch::tensor({{0.1}, {0.2}, {0.3}, {0.4}}, torch::TensorOptions().dtype(torch::kDouble));
  batch.log_probabilities =
      torch::tensor({{-1.0}, {-1.1}, {-1.2}, {-1.3}}, torch::TensorOptions().dtype(torch::kDouble));
  batch.value_targets =
      torch::tensor({{1.0}, {2.0}, {3.0}, {4.0}}, torch::TensorOptions().dtype(torch::kDouble));
  batch.advantages =
      torch::tensor({{1.0}, {2.0}, {3.0}, {4.0}}, torch::TensorOptions().dtype(torch::kDouble));

  LibtorchRLMiniBatchSampler sampler;
  const auto first_batches = sampler.sample(batch, 2, false, Moose::makeLibtorchCPUGenerator(9876));
  const auto second_batches =
      sampler.sample(batch, 2, false, Moose::makeLibtorchCPUGenerator(9876));

  ASSERT_EQ(first_batches.size(), second_batches.size());
  for (const auto i : index_range(first_batches))
  {
    EXPECT_TRUE(torch::allclose(first_batches[i].observations,
                                second_batches[i].observations,
                                /* rtol = */ 0.0,
                                /* atol = */ 0.0));
    EXPECT_TRUE(torch::allclose(first_batches[i].actions,
                                second_batches[i].actions,
                                /* rtol = */ 0.0,
                                /* atol = */ 0.0));
  }
}

} // namespace

#endif

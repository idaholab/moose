//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_LIBTORCH_ENABLED

#include "LibtorchRLValueEstimator.h"

#include "LibtorchUtils.h"

#include "libmesh/utility.h"

namespace
{

torch::Tensor
valueEstimatorMatrixToTensor(const std::vector<std::vector<Real>> & rows)
{
  if (rows.empty())
    return {};

  const auto num_columns = rows.front().size();
  auto tensor = torch::zeros({static_cast<long>(rows.size()), static_cast<long>(num_columns)},
                             torch::TensorOptions().dtype(torch::kDouble));
  auto accessor = tensor.accessor<Real, 2>();

  for (const auto row_i : make_range(rows.size()))
    for (const auto column_i : make_range(num_columns))
      accessor[row_i][column_i] = rows[row_i][column_i];

  return tensor;
}

} // namespace

LibtorchRLValueEstimator::LibtorchRLValueEstimator(const Real discount_factor,
                                                   const Real lambda_factor)
  : _discount_factor(discount_factor), _lambda_factor(lambda_factor)
{
}

void
LibtorchRLValueEstimator::computeValueTargets(
    LibtorchRLTrajectoryBuffer & buffer, Moose::LibtorchArtificialNeuralNet & value_network) const
{
  for (auto & trajectory : buffer.trajectories())
  {
    const auto targets = estimate(trajectory, value_network);
    trajectory.advantages = targets.advantages;
    trajectory.value_targets = targets.value_targets;
  }
}

LibtorchRLValueEstimator::Targets
LibtorchRLValueEstimator::estimate(const LibtorchRLTrajectoryBuffer::Trajectory & trajectory,
                                   Moose::LibtorchArtificialNeuralNet & value_network) const
{
  Targets targets;

  const auto values = evaluate(trajectory.observations, value_network);
  const auto next_values = evaluate(trajectory.next_observations, value_network);

  const auto num_steps = trajectory.rewards.size();
  targets.advantages.resize(num_steps, 0.0);
  targets.value_targets.resize(num_steps, 0.0);

  Real gae = 0.0;
  for (const auto reverse_step : make_range(num_steps))
  {
    const auto step = num_steps - reverse_step - 1;
    const auto delta =
        trajectory.rewards[step] + _discount_factor * next_values[step] - values[step];
    gae = delta + _discount_factor * _lambda_factor * gae;
    targets.advantages[step] = gae;
    targets.value_targets[step] = gae + values[step];
  }

  return targets;
}

std::vector<Real>
LibtorchRLValueEstimator::evaluate(const std::vector<std::vector<Real>> & observations,
                                   Moose::LibtorchArtificialNeuralNet & value_network) const
{
  if (observations.empty())
    return {};

  torch::NoGradGuard no_grad;

  auto tensor = valueEstimatorMatrixToTensor(observations);
  auto value_tensor = value_network.forward(tensor);
  auto flattened_value_tensor = value_tensor.reshape({-1});

  std::vector<Real> values;
  LibtorchUtils::tensorToVector(flattened_value_tensor, values);
  return values;
}

#endif

//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_LIBTORCH_ENABLED

#pragma once

#include "LibtorchObservationHistoryHelper.h"
#include "LibtorchRLTrajectoryBuffer.h"

#include <string>
#include <vector>

/**
 * Builds fixed-horizon RL trajectories from raw per-sample reporter sequences.
 *
 * Raw index 0 of every sequence holds the initial value and raw index n holds the value at the
 * end of time step n. Transition k uses the observation at raw index k * W, the action and log
 * probability at raw index k * W (+ 1 when outputs are shifted), and the reward at raw index
 * (k + 1) * W, or the mean over raw indices k * W + 1 through (k + 1) * W. Here W is the
 * timestep window. Observation history lags also span W raw entries.
 */
class LibtorchRLTrajectoryAssembler
{
public:
  /// Raw reporter data for one rollout quantity, indexed as [component][sample][raw time].
  using ReporterData = std::vector<const std::vector<std::vector<Real>> *>;

  /**
   * Build the trajectory assembler.
   * @param input_timesteps Number of stacked observation time levels.
   * @param timestep_window Raw reporter entries per transition.
   * @param shift_outputs Whether actions and log probabilities are read one raw entry later.
   * @param average_reward_over_timestep_window Whether rewards are averaged over each window.
   */
  LibtorchRLTrajectoryAssembler(unsigned int input_timesteps,
                                unsigned int timestep_window,
                                bool shift_outputs,
                                bool average_reward_over_timestep_window);

  /**
   * Assemble one trajectory per sample that holds at least one transition.
   * @param observations Observation reporter data.
   * @param actions Action reporter data.
   * @param log_probabilities Log-probability reporter data, one entry per action.
   * @param rewards Reward reporter data indexed as [sample][raw time].
   * @return Assembled trajectories in sample order.
   */
  std::vector<LibtorchRLTrajectoryBuffer::Trajectory>
  assemble(const ReporterData & observations,
           const ReporterData & actions,
           const ReporterData & log_probabilities,
           const std::vector<std::vector<Real>> & rewards) const;

  /**
   * Count the transitions available in a raw reporter sequence.
   * @param raw_sequence_size Number of raw time entries in the reporter sequence.
   * @return Number of complete timestep windows after the initial entry.
   */
  unsigned int numTransitions(std::size_t raw_sequence_size) const;

private:
  /**
   * Check that every component of a rollout quantity holds one sequence per sample.
   * @param data Reporter data to check.
   * @param quantity Name of the rollout quantity used in the error message.
   * @param num_samples Number of samples held by the reward reporter.
   */
  static void checkSampleCount(const ReporterData & data,
                               const std::string & quantity,
                               std::size_t num_samples);

  /**
   * Take every timestep_window-th entry of a raw reporter sequence.
   * @param sample Raw reporter sequence.
   * @param offset Raw index of the first entry.
   * @param num_entries Number of entries to extract.
   * @return Downsampled sequence.
   */
  std::vector<Real>
  downsample(const std::vector<Real> & sample, unsigned int offset, unsigned int num_entries) const;

  /**
   * Average a raw reporter sequence over consecutive timestep windows after the initial entry.
   * @param sample Raw reporter sequence.
   * @param num_entries Number of windows to average.
   * @return Window-averaged sequence.
   */
  std::vector<Real> windowAverage(const std::vector<Real> & sample, unsigned int num_entries) const;

  /// Raw reporter entries per transition.
  const unsigned int _timestep_window;
  /// Whether actions and log probabilities are read one raw entry after the observation.
  const bool _shift_outputs;
  /// Whether rewards are averaged over each window instead of sampled at its end.
  const bool _average_reward_over_timestep_window;
  /// Observation history stacking helper.
  const LibtorchObservationHistoryHelper _observation_history;
};

#endif

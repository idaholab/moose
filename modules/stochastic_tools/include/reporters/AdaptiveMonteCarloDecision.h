//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralReporter.h"
#include "AdaptiveImportanceSampler.h"
#include "ParallelSubsetSimulation.h"

/**
 * AdaptiveMonteCarloDecision will help make sample accept/reject decisions in adaptive Monte Carlo
 * schemes.
 */
class AdaptiveMonteCarloDecision : public GeneralReporter
{
public:
  static InputParameters validParams();
  AdaptiveMonteCarloDecision(const InputParameters & parameters);
  virtual void initialize() override {}
  virtual void finalize() override {}
  virtual void execute() override;

protected:
  /// Model output value from SubApp
  const std::vector<Real> & _output_value;

  /// Modified value of model output by this reporter class
  std::vector<Real> & _output_required;

  /// Model input data that is uncertain
  std::vector<std::vector<Real>> & _inputs;

private:
  /// Track the current step of the main App
  const int & _step;

  /// The adaptive Monte Carlo sampler
  Sampler & _sampler;

  /// Adaptive Importance Sampler
  const AdaptiveImportanceSampler * const _ais;

  /// Parallel Subset Simulation sampler
  const ParallelSubsetSimulation * const _pss;

  /// Ensure that the MCMC algorithm proceeds in a sequential fashion
  int _check_step;

  /// Communicator that was split based on samples that have rows
  libMesh::Parallel::Communicator _local_comm;

  /// Storage for previously accepted input values. This helps in making decision on the next proposed inputs.
  std::vector<std::vector<Real>> _prev_val;

  /// Storage for previously accepted output value.
  std::vector<Real> _prev_val_out;

  /// Storage for the previously accepted sample inputs across all the subsets
  std::vector<std::vector<Real>> _inputs_sto;

  /// Store the sorted input samples according to their corresponding outputs
  std::vector<std::vector<Real>> _inputs_sorted;

  /// Storage for previously accepted sample outputs across all the subsets
  std::vector<Real> _outputs_sto;

  /// Store the sorted output sample values
  std::vector<Real> _output_sorted;

  /// Store the intermediate ouput failure thresholds
  Real _output_limit;
};

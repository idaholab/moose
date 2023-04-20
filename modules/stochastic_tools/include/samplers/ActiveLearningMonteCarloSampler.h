//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Sampler.h"

/**
 * A class used to perform Monte Carlo Sampling with active learning
 */
class ActiveLearningMonteCarloSampler : public Sampler, public ReporterInterface
{
public:
  static InputParameters validParams();

  ActiveLearningMonteCarloSampler(const InputParameters & parameters);

  /**
   * Returns true if the adaptive sampling is completed
   */
  virtual bool isAdaptiveSamplingCompleted() const override { return _is_sampling_completed; }

protected:
  /// Gather all the samples
  virtual void sampleSetUp(const Sampler::SampleMode mode) override;
  /// Return the sample for the given row and column
  virtual Real computeSample(dof_id_type row_index, dof_id_type col_index) override;

  /// Storage for distribution objects to be utilized
  std::vector<Distribution const *> _distributions;

  /// Flag samples if the surrogate prediction was inadequate
  const std::vector<bool> & _flag_sample;

  /// True if the sampling is completed
  bool _is_sampling_completed = false;

private:
  /// Track the current step of the main App
  const int & _step;

  /// The maximum number of GP fails
  const unsigned int _num_batch;

  /// Ensure that the sampler proceeds in a sequential fashion
  int _check_step;

  /// Number of samples requested
  const int & _num_samples;

  /// Number of retraining performed
  int _retraining_steps = 0;

  /// Storage for previously accepted samples by the decision reporter system
  std::vector<std::vector<Real>> _inputs_sto;

  /// Store the input params for which the GP fails
  std::vector<std::vector<Real>> _inputs_gp_fails;
};

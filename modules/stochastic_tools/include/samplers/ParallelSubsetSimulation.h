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
#include "ReporterInterface.h"

/**
 * A class used to perform Parallel Subset Simulation Sampling
 */
class ParallelSubsetSimulation : public Sampler, public ReporterInterface
{
public:
  static InputParameters validParams();

  ParallelSubsetSimulation(const InputParameters & parameters);

  /// Access the number samples per subset
  const unsigned int & getNumSamplesSub() const;

  /// Access use absolute value bool
  const bool & getUseAbsoluteValue() const;

  /// Access the subset probability
  const Real & getSubsetProbability() const;

  /**
   * Returns true if the adaptive sampling is completed
   */
  virtual bool isAdaptiveSamplingCompleted() const override { return _is_sampling_completed; }

protected:
  virtual void sampleSetUp(const Sampler::SampleMode mode) override;
  virtual Real computeSample(dof_id_type row_index, dof_id_type col_index) override;

  /// Number of samples per subset
  const unsigned int & _num_samplessub;

  /// Number of subsets
  const unsigned int & _num_subsets;

  /// Absolute value of the model result. Use this when failure is defined as a non-exceedance rather than an exceedance.
  const bool & _use_absolute_value;

  /// The subset conditional failure probability
  const Real & _subset_probability;

  /// Initialize a certain number of random seeds. Change from the default only if you have to.
  const unsigned int & _num_random_seeds;

  /// Reporter value containing calculated outputs
  const std::vector<Real> & _outputs;

  /// Reporter value containing input values from decision reporter
  const std::vector<std::vector<Real>> & _inputs;

  /// Track the current step of the main App
  const int & _step;

  /// Maximum length of markov chains based on subset probability
  const unsigned int _count_max;

  /// Ensure that the MCMC algorithm proceeds in a sequential fashion
  int _check_step;

  /// Track the current subset index
  unsigned int _subset;

  /// Storage for distribution objects to be utilized
  std::vector<Distribution const *> _distributions;

  /// True if the sampling is completed
  bool _is_sampling_completed;

private:
  /// Storage for the previously accepted sample inputs across all the subsets
  std::vector<std::vector<Real>> _inputs_sto;

  /// Storage for previously accepted sample outputs across all the subsets
  std::vector<Real> _outputs_sto;

  /// Store the sorted input samples according to their corresponding outputs
  std::vector<std::vector<Real>> _inputs_sorted;

  /// Mean input vector for the next proposed sample inputs across several processors
  std::vector<std::vector<Real>> _markov_seed;
};

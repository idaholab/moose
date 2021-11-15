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
  const int & getNumSamplesSub() const;

  /// Access use absolute value bool
  const bool & getUseAbsoluteValue() const;

  /// Access the subset probability
  const Real & getSubsetProbability() const;

protected:
  /// Return the sample for the given row and column
  virtual Real computeSample(dof_id_type row_index, dof_id_type col_index) override;

  /// Storage for distribution objects to be utilized
  std::vector<Distribution const *> _distributions;

  /// Number of samples per subset
  const int & _num_samplessub;

  /// Absolute value of the model result. Use this when failure is defined as a non-exceedance rather than an exceedance.
  const bool & _use_absolute_value;

  /// The subset conditional failure probability
  const Real & _subset_probability;

  /// Initialize a certain number of random seeds. Change from the default only if you have to.
  const unsigned int & _num_random_seeds;

private:

  /// Storage for the previously accepted sample inputs across all the subsets
  std::vector<std::vector<Real>> _inputs_sto;

  /// Storage for previously accepted sample outputs across all the subsets
  std::vector<Real> _outputs_sto;

  /// Storage for the next proposed sample inputs across all the processors
  std::vector<std::vector<Real>> _new_sample_vec;

  /// Storage for the acceptance ratio for the MCMC sampler
  Real _acceptance_ratio;

  /// Track the current step of the main App
  const int & _step;

  /// Track the current subset index
  unsigned int _subset;

  /// Ensure proper randomization across several processors in parallel
  unsigned int _seed_value;

  /// Aid in proposing the next sample inputs across several processors in parallel
  int _ind_sto;

  /// Mean input vector for the next proposed sample inputs across several processors
  std::vector<std::vector<Real>> _markov_seed;

  /// Aid in proposing the next sample inputs across several processors in parallel
  unsigned int _count;

  /// Ensure that the MCMC algorithm proceeds in a sequential fashion
  int _check_step;

  /// Aid in selecting the seed input values for proposing the next input sample
  unsigned int _count_max;

  /// Store the sorted input samples according to their corresponding outputs
  std::vector<std::vector<Real>> _inputs_sorted;

  /// Storage of the previous sample to propose the next sample
  std::vector<Real> _prev_val;

  /// Store the intermediate ouput failure thresholds
  std::vector<Real> _output_limits;
};
